#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

#include "Core/Hooks.h"

#include "Combat/Block.h"
#include "Combat/Execution.h"
#include "Combat/Exhausted.h"
#include "Combat/Posture.h"
#include "Combat/Stagger.h"
#include "Combat/WeaponArt.h"
#include "GUI/UI.h"
#include "Utils.h"

#include "detours.h"
#include "magic_enum/magic_enum.hpp"

namespace Hooks
{
// 辅助函数
namespace
{
  bool NeedDisableAttack(RE::Actor* actor)
  {

    if (!actor)
      return false;

    if (actor->IsPlayerRef()) {
      if (Settings::bDisablePlayerAttackWhenStaminaZero &&
          actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) <= 0)
        return true;

      if (Settings::bDisablePlayerAttackWhenExhausted && Exhausted::IsActorExhausted(actor))
        return true;
    } else {
      if (Settings::bDisableNPCAttackWhenExhausted && Exhausted::IsActorExhausted(actor))
        return true;
    }

    if (Execution::IsExecutable(actor))
      return true;

    return false;
  };
}  // namespace

void Hook_OnMainUpdate::MainUpdate()
{
  _MainUpdate();

  static std::uint64_t now       = 0;
  static std::uint64_t deltaTime = 0;

  if (!now)
    now = Utils::GetTime<std::chrono::milliseconds>();

  deltaTime = Utils::GetTime<std::chrono::milliseconds>() - now;
  now       = Utils::GetTime<std::chrono::milliseconds>();

  // 更新Utils提供的主线程接口
  Utils::MainUpdate();

  // 更新战技菜单选中物品同步
  UI::WeaponArtMenu::Update();

  // 更新格挡系统
  Block::Update();

  // 更新架势系统
  Posture::Update(deltaTime);

  // 更新处决系统
  Execution::Update();
}

float Hook_OnGetAttackStaminaCost::GetAttackStaminaCost(RE::ActorValueOwner* avOwner,
                                                        RE::BGSAttackData* atkData)
{
  // 如果启用了攻击耐力系统，则取消掉原版的攻击耐力消耗
  if (Settings::bUseAttackStaminaSystem)
    return 0.0f;

  return _GetAttackStaminaCost(avOwner, atkData);
}

void Hook_OnMeleeHit::ProcessHit(RE::Actor* victim, RE::HitData& hitData)
{
  auto* aggressor = hitData.aggressor.get().get();

  if (!victim || !aggressor || victim->IsDead())
    return _ProcessHit(victim, hitData);

  // 第一部分：根据状态修正数值

  // 处决状态增伤
  if (Settings::bExitExecutionOnHit && Execution::IsExecutable(victim))
    hitData.totalDamage *= Settings::fOnHitDamageMultWhenExecutable;

  // 力竭状态增伤
  if (Settings::bExitExhaustedOnHit && Exhausted::IsActorExhausted(victim))
    hitData.totalDamage *= Settings::fOnHitDamageMultWhenExhausted;

  // 第二部分：处理各个模块的攻击处理

  if (hitData.flags.any(RE::HitData::Flag::kBlocked)) {
    Block::ProcessBlock(victim);
    Block::ProcessDamage(victim, hitData);
  }

  Posture::ProcessMeleeHit(aggressor, victim, hitData);

  // 韧性相关的模组都会在处理攻击之中调用TryStagger
  // 不能保证对硬直等级的修改时序在他们修改之前，TryStagger之后
  // 因此直接Detour TryStagger

  // 战技经验应该是根据最终伤害来计算的，所以放在最后处理
  if (aggressor->IsPlayerRef() && WeaponArt::Manager::IsPerforming(aggressor))
    WeaponArt::PlayerStat::AddExp(hitData.totalDamage);

  // 第二阶段的末尾，在所有处理完成后再调用原版的ProcessHit，确保所有状态和数值都已经更新完毕
  _ProcessHit(victim, hitData);

  // 第三部分：根据设置退出状态
  // 因为非即时的退出，可能导致更新带来的状态变更
  // 但因为在同一个函数且逻辑不重
  // 1ms的延迟应该不会带来明显的问题，同时也能保证状态变更的正确性

  // 如果设置处决状态被击打时退出，则在此处退出
  if (Settings::bExitExecutionOnHit && Execution::IsExecutable(victim))
    Execution::ExitExecutable(victim);

  // 如果设置了被击打时退出力竭状态，则在此处退出
  if (Settings::bExitExhaustedOnHit && Exhausted::IsActorExhausted(victim))
    Exhausted::ExitExhausted(victim);
}

void Hook_OnTryStagger::Install()
{
  std::uintptr_t addr = REL::VariantID(36700, 37710, 0).address();
  _TryStagger         = reinterpret_cast<decltype(_TryStagger)>(addr);

  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(reinterpret_cast<PVOID*>(&_TryStagger), TryStagger);
  if (DetourTransactionCommit() != NO_ERROR) {
    logger::error("Failed to install Hook_OnTryStagger.");
    return;
  }
  logger::info("Hook: OnTryStagger installed.");
}
void Hook_OnTryStagger::TryStagger(RE::Actor* target, float staggerMult, RE::Actor* aggressor)
{
  // 使用Detour的hook模式取得最低的调用优先级，确保在其他模组修改硬直等级之后再进行计算和处理
  // TryStagger的第二个参数会被原版用于回写staggerMagnitude
  // 因此需要在这里统一修正传给原版的最终数值，而不只是修改图变量
  auto res = Stagger::ProcessStagger(aggressor, target);

  if (res)
    _TryStagger(target, Stagger::GetStaggerMagnitude(target), aggressor);
  else
    _TryStagger(target, staggerMult, aggressor);
}

void Hook_OnPlayIdle::Install()
{
  std::uintptr_t addr = REL::VariantID(38290, 39256, 0).address();
  _PlayIdle           = reinterpret_cast<decltype(_PlayIdle)>(addr);

  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(reinterpret_cast<PVOID*>(&_PlayIdle), PlayIdle);
  if (DetourTransactionCommit() != NO_ERROR) {
    logger::error("Failed to install Hook_OnPlayIdle.");
    return;
  }
  logger::info("Hook: OnPlayIdle installed.");
}
bool Hook_OnPlayIdle::PlayIdle(RE::AIProcess* aiProcess, RE::Actor* actor,
                               RE::DEFAULT_OBJECT action, RE::TESIdleForm* idle, bool arg5,
                               bool arg6, RE::TESObjectREFR* target)
{
  // 仅针对玩家
  if (!aiProcess || !actor || !idle)
    return _PlayIdle(aiProcess, actor, action, idle, arg5, arg6, target);

  // 只有当执行攻击相关的Idle时才进行干预
  std::string animEvent = idle->animEventName.data();
  std::transform(animEvent.begin(), animEvent.end(), animEvent.begin(), ::tolower);
  auto animEventHash = Utils::hash(animEvent);

  switch (animEventHash) {
  case "attackstart"_h:
  case "attackpowerstartinplace"_h:
  case "attackstartsprint"_h:
  case "attackpowerstart_sprint"_h:
    if (NeedDisableAttack(actor))
      return false;
  default:
    return _PlayIdle(aiProcess, actor, action, idle, arg5, arg6, target);
  }
}

void Hook_OnPerformAction::Install()
{
  std::uintptr_t addr = REL::VariantID(40551, 41557, 0).address();
  _PerformAction      = reinterpret_cast<decltype(_PerformAction)>(addr);

  DetourTransactionBegin();
  DetourUpdateThread(GetCurrentThread());
  DetourAttach(reinterpret_cast<PVOID*>(&_PerformAction), PerformAction);
  if (DetourTransactionCommit() != NO_ERROR) {
    logger::error("Failed to install Hook_OnPerformAction.");
    return;
  }
  logger::info("Hook: OnPerformAction installed.");
}

bool Hook_OnPerformAction::PerformAction(RE::TESActionData* actionData)
{
  if (!actionData)
    return _PerformAction(actionData);

  auto actorState = actionData->GetSourceActorState();
  if (!actorState)
    return _PerformAction(actionData);

  RE::Actor* sourceActor = skyrim_cast<RE::Actor*>(actorState);
  if (!sourceActor)
    return _PerformAction(actionData);

  auto action = actionData->action;
  if (!action)
    return _PerformAction(actionData);

  switch (action->GetFormID()) {
    // MCO/BFCO框架下的攻击
    // 对于Right，一般都是真正的攻击动作
    // 对于Left，一般都是防御动作
    // 对于Dual，几乎用不到，暂时不处理
  case 0x13005:  // ActionRightAttack
  case 0x13383:  // ActionRightPowerAttack

    if (auto victim = Execution::FindExecutableTarget(sourceActor); victim) {
      // 如果找到了可处决的目标，则强制进入处决处决判断
      // 如果判断成功取消攻击进入处决动作
      // 否则正常执行攻击动作
      if (Execution::TryExecute(sourceActor, victim))
        return false;
    }

    if (NeedDisableAttack(sourceActor))
      return false;
    break;

    // case 0x13004:  // ActionLeftAttack
    // case 0x2E2F6:  // ActionLeftPowerAttack
    // case 0x50C96:  // ActionDualAttack
    // case 0x2E2F7:  // ActionDualPowerAttack

  case 0x13AF5:  // ActionRecoil
    if (Block::IsBlocking(sourceActor))
      return false;
    break;
  case 0x13EC8:  // ActionRecoilLarge
    if (Block::IsTimedBlocking(sourceActor))
      return false;
    break;
  case 0x138D2:  // ActionStaggerStart
  {
    // 如果硬直等级不足或者硬直免疫
    // 把RimCombat的硬直系统的硬直等级重置为0
    // 避免因为没有进入硬直动作导致的RimCombat的硬直系统被意外触发
    auto staggerLevel = Stagger::GetStaggerLevel(sourceActor);
    if (staggerLevel < Stagger::Level::Largest)
      sourceActor->SetGraphVariableInt(Stagger::STAGGER_LEVEL, 0);

    if (Block::IsTimedBlocking(sourceActor) || Stagger::IsStaggerImmune(sourceActor)) {
      sourceActor->SetGraphVariableInt(Stagger::STAGGER_LEVEL, 0);
      return false;
    }

    break;
  }
  case 0x13002:  // ActionIdle
    // 对于idle，转交给PlayIdle的hook来处理
    break;

  default:
    // 其他类型不处理
    break;
  }

  return _PerformAction(actionData);
}

void Hook_OnModActorValue::ModActorValue_NPC(RE::ActorValueOwner* avOwner,
                                             RE::ACTOR_VALUE_MODIFIER modifier,
                                             RE::ActorValue akValue, float value)
{
  RE::Actor* actor = skyrim_cast<RE::Actor*>(avOwner);
  value            = ModActorValue(actor, modifier, akValue, value);
  _ModActorValue_NPC(avOwner, modifier, akValue, value);
}
void Hook_OnModActorValue::ModActorValue_PC(RE::ActorValueOwner* avOwner,
                                            RE::ACTOR_VALUE_MODIFIER modifier,
                                            RE::ActorValue akValue, float value)
{
  RE::Actor* actor = skyrim_cast<RE::Actor*>(avOwner);
  value            = ModActorValue(actor, modifier, akValue, value);
  _ModActorValue_PC(avOwner, modifier, akValue, value);
}
float Hook_OnModActorValue::ModActorValue(RE::Actor* actor, RE::ACTOR_VALUE_MODIFIER modifier,
                                          RE::ActorValue akValue, float value)
{
  switch (modifier) {
  case RE::ACTOR_VALUE_MODIFIER::kPermanent:
  case RE::ACTOR_VALUE_MODIFIER::kTemporary:
    return ModMaxActorValue(actor, akValue, value);
  case RE::ACTOR_VALUE_MODIFIER::kDamage:
    return ModCurrentActorValue(actor, akValue, value);
  default:
    return value;
  }
}
float Hook_OnModActorValue::ModMaxActorValue(RE::Actor* actor, RE::ActorValue akValue, float value)
{
  switch (akValue) {
  case RE::ActorValue::kHealth:
    Posture::ReCalculateMaxPosture(actor);
    break;
  }
  return value;
}
float Hook_OnModActorValue::ModCurrentActorValue(RE::Actor* actor, RE::ActorValue akValue,
                                                 float value)
{
  // Process ActorValue Decrease
  if (value < 0) {
    // Process Exhausted State Entry
    if (akValue == RE::ActorValue::kStamina) {
      // 判断当前Actor的耐力如果即将降到0或以下，则进入疲惫状态
      if (actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) + value <= 0)
        Exhausted::EnterExhausted(actor);
    }
    // Process player god mode
    if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode()) {
      switch (akValue) {
      case RE::ActorValue::kHealth:
      case RE::ActorValue::kStamina:
      case RE::ActorValue::kMagicka:
        return 0.0f;
      default:
        break;
      }
    }
    return value;
  }
  // Process ActorValue Increase
  switch (akValue) {
  case RE::ActorValue::kStamina: {
    // During attack, stamina does not regenerate
    RE::ATTACK_STATE_ENUM atkState = actor->AsActorState()->GetAttackState();
    if (atkState > RE::ATTACK_STATE_ENUM::kNone &&
        atkState <= RE::ATTACK_STATE_ENUM::kBowFollowThrough) {
      return 0.0f;
    }
    // Combat Multiplier
    if (actor->IsInCombat()) {
      value *= Settings::fStaminaRegenMultCombat;
    }
    // Block Multiplier
    if (actor->IsBlocking()) {
      value *= Settings::fStaminaRegenMultBlock;
    }
    // Process Exhausted State Exit
    if (Exhausted::IsActorExhausted(actor)) {
      if (Utils::GetCurrentActorValuePercent(actor, RE::ActorValue::kStamina) >=
          Settings::fExhaustedExitPercent)
        Exhausted::ExitExhausted(actor);
    }
  } break;
  default:
    break;
  }
  return value;
}

// 在更换装备时更新当前战技ID
void Hook_OnEquipObject::OnEquipObject(RE::ActorEquipManager* manager, RE::Actor* actor,
                                       RE::TESBoundObject* object, std::uint64_t unk)
{
  _OnEquipObject(manager, actor, object, unk);
  WeaponArt::Manager::UpdateWeaponArt(actor);
}
void Hook_OnUnequipObject::OnUnequipObject(RE::ActorEquipManager* manager, RE::Actor* actor,
                                           RE::TESBoundObject* object, std::uint64_t unk)
{
  _OnUnequipObject(manager, actor, object, unk);
  WeaponArt::Manager::UpdateWeaponArt(actor);
}

void Install()
{
  Hook_OnMainUpdate::Install();
  Hook_OnActorUpdate::Install();
  Hook_OnGetAttackStaminaCost::Install();
  Hook_OnMeleeHit::Install();
  Hook_OnTryStagger::Install();
  Hook_OnPlayIdle::Install();
  Hook_OnPerformAction::Install();
  Hook_OnModActorValue::Install();
  Hook_OnEquipObject::Install();
  Hook_OnUnequipObject::Install();
}
}  // namespace Hooks