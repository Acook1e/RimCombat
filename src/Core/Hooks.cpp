#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmicrosoft-cast"

#include "Core/Hooks.h"

#include "Combat/Block.h"
#include "Combat/Damage.h"
#include "Combat/Execution.h"
#include "Combat/Exhausted.h"
#include "Combat/Poise.h"
#include "Combat/Posture.h"
#include "Combat/Stagger.h"
#include "Combat/Stamina.h"
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

    if (Settings::bDisableAttackWhenStaminaZero &&
        actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) <= 0)
      return true;

    if (actor->IsPlayerRef()) {
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

  // 更新格挡系统
  Block::Update();

  // 更新处决系统
  Execution::Update();

  // 更新硬直持续状态
  Stagger::Update();

  // 更新韧性系统
  Poise::Update(deltaTime);

  // 更新架势系统
  Posture::Update(deltaTime);

  // 更新战技菜单选中物品同步
  UI::WeaponArtMenu::Update();

  // 更新Utils提供的主线程接口
  Utils::MainUpdate();
}

void Hook_OnActorUpdate::Update_NPC(RE::Character* character, float delta)
{
  _Update_NPC(character, delta);
  TrackActorUpdate(character);
}

void Hook_OnActorUpdate::Update_PC(RE::PlayerCharacter* player, float delta)
{
  _Update_PC(player, delta);
  TrackActorUpdate(player);
}

void Hook_OnActorUpdate::TrackActorUpdate(RE::Actor* actor)
{
  static std::unordered_map<RE::Actor*, RE::ATTACK_STATE_ENUM> lastAttackStates{};

  if (!actor || !actor->Is3DLoaded() || actor->IsDead())
    return;

  auto attackState = actor->AsActorState()->GetAttackState();

  if (!lastAttackStates.contains(actor))
    lastAttackStates[actor] = attackState;
  else {
    auto lastState = lastAttackStates[actor];

    if (attackState != lastState) {
      if (Race::GetRace(actor) != Race::Type::Human) {
        logger::info("Race {} AttackState changed: {} -> {}",
                     magic_enum::enum_name(Race::GetRace(actor)), magic_enum::enum_name(lastState),
                     magic_enum::enum_name(attackState));
      } else {
        if (attackState == RE::ATTACK_STATE_ENUM::kSwing) {
          auto weapon = actor->GetAttackingWeapon();
          if (weapon && weapon->object && weapon->object->IsWeapon())
            Stamina::SwingStaminaConsume(actor, weapon->object->As<RE::TESObjectWEAP>());
        } else if (attackState == RE::ATTACK_STATE_ENUM::kBash)
          Stamina::BashStaminaConsume(actor);
      }
      lastAttackStates[actor] = attackState;
    }
  }
}

float Hook_OnGetAttackStaminaCost::GetAttackStaminaCost(RE::ActorValueOwner* avOwner,
                                                        RE::BGSAttackData* atkData)
{
  // 如果启用了攻击耐力系统，则取消掉原版的攻击耐力消耗
  if (Settings::bUseAttackStaminaSystem)
    return 0.0f;

  return _GetAttackStaminaCost(avOwner, atkData);
}

float Hook_OnGetMeleeDamage::GetWeaponDamage(RE::InventoryEntryData* weapon,
                                             RE::ActorValueOwner* actorValueOwner, float damageMult,
                                             bool unk)
{
  float damage = _GetWeaponDamage(weapon, actorValueOwner, damageMult, unk);

  auto actor = skyrim_cast<RE::Actor*>(actorValueOwner);
  if (actor)
    Damage::ProcessDamage(actor, damage);
  return damage;
}

void Hook_OnGetMeleeDamage::GetBashDamage(RE::ActorValueOwner* actorValueOwner, float& outDamage)
{
  _GetBashDamage(actorValueOwner, outDamage);

  auto actor = skyrim_cast<RE::Actor*>(actorValueOwner);
  if (actor)
    Damage::ProcessDamage(actor, outDamage);
}

void Hook_OnGetMeleeDamage::GetUnarmedDamage(RE::ActorValueOwner* actorValueOwner, float& outDamage)
{
  _GetUnarmedDamage(actorValueOwner, outDamage);

  auto actor = skyrim_cast<RE::Actor*>(actorValueOwner);
  if (actor)
    Damage::ProcessDamage(actor, outDamage);
}

void Hook_OnWeaponHit::ProcessHit(RE::Actor* victim, RE::HitData& hitData)
{
  auto* aggressor = hitData.aggressor.get().get();

  if (!victim || !aggressor || victim->IsDead())
    return _ProcessHit(victim, hitData);

  // 第一部分：根据状态修正数值

  Damage::ProcessWeaponDamage(aggressor, hitData);

  // 处决状态增伤
  // 引入处决触发的入口在Posture，退出状态必须在Posture之前
  if (Execution::IsExecutable(victim)) {
    hitData.totalDamage *= Settings::fOnHitDamageMultWhenExecutable;
    Execution::ExitExecutable(victim);
    victim->NotifyAnimationGraph("staggerStop");
  }

  // 力竭状态伤害变动
  if (Settings::fAttackDamageMultWhenExhausted > 0.0f && Exhausted::IsActorExhausted(aggressor))
    hitData.totalDamage *= Settings::fAttackDamageMultWhenExhausted;

  if (Settings::fOnHitDamageMultWhenExhausted > 0.0f && Exhausted::IsActorExhausted(victim))
    hitData.totalDamage *= Settings::fOnHitDamageMultWhenExhausted;

  // 战技经验应该是根据最终伤害来计算的，放在第一部分最后处理
  if (aggressor->IsPlayerRef() &&
      WeaponArt::Manager::GetPerform(aggressor) != WeaponArt::Manager::Perform::None)
    WeaponArt::PlayerStat::AddExp(hitData.totalDamage);

  // 第二部分：处理会直接修改HitData的状态和数值变更

  // 如果使用Rim Combat的硬直系统，则在这里将原版的硬直数值清零，避免被原版的硬直系统再次处理
  if (Settings::bUseStaggerSystem)
    hitData.stagger = 0.0f;

  Block::ProcessBlock(aggressor, victim, hitData);

  // 在架势之前处理，处决级别的硬直入口在架势
  // 格挡命中不处理韧性，0伤害命中不处理韧性/架势
  Poise::ProcessWeaponHit(aggressor, victim, hitData);

  Posture::ProcessWeaponHit(aggressor, victim, hitData);

  _ProcessHit(victim, hitData);

  // 第三部分：处理会修改状态但不直接修改HitData的变更
  // 此时HitData中的数值已经是最终的伤害，可以根据这个数值来处理一些状态变更

  // 在所有涉及硬直的系统的最后处理
  Stagger::ProcessWeaponStagger(aggressor, victim);

  // 如果设置了被击打时退出力竭状态，则在此处退出
  if (Settings::bExitExhaustedOnHit && Exhausted::IsActorExhausted(victim))
    Exhausted::ExitExhausted(victim);
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
  // 对于Dual，是BFCO的特殊攻击
  case 0x13005:  // ActionRightAttack

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

    // case 0x13383:  // ActionRightPowerAttack
    // case 0x50C96:  // ActionDualAttack
    // case 0x2E2F7:  // ActionDualPowerAttack
    // case 0x13004:  // ActionLeftAttack
    // case 0x2E2F6:  // ActionLeftPowerAttack

  case 0x959F8:  // ActionMoveStart
  case 0x5EDC9:  // ActionMoveForward
  case 0x5EDCC:  // ActionMoveBackward
  case 0x5EDCD:  // ActionMoveLeft
  case 0x5EDCE:  // ActionMoveRight
  case 0x959FC:  // ActionTurnLeft
  case 0x959FD:  // ActionTurnRight
  case 0x13006:  // ActionJump
    break;
  case 0x13AF5:  // ActionRecoil
    if (Block::IsBlocking(sourceActor))
      return false;
    break;
  case 0x13EC8:  // ActionRecoilLarge
    if (Block::IsTimedBlocking(sourceActor))
      return false;
    break;
  case 0x138D2:  // ActionStaggerStart
    break;
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
    Posture::UpdateMaxPosture(actor);
    break;
  case RE::ActorValue::kStamina:
    Posture::UpdateMaxPosture(actor);
    Poise::UpdateMaxPoise(actor);
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

// 在更换装备时更新
void Hook_OnEquipObject::OnEquipObject(RE::ActorEquipManager* manager, RE::Actor* actor,
                                       RE::TESBoundObject* object, std::uint64_t unk)
{
  _OnEquipObject(manager, actor, object, unk);
  Poise::UpdateMaxPoise(actor);
  WeaponArt::Manager::UpdateWeaponArt(actor);
}
void Hook_OnUnequipObject::OnUnequipObject(RE::ActorEquipManager* manager, RE::Actor* actor,
                                           RE::TESBoundObject* object, std::uint64_t unk)
{
  _OnUnequipObject(manager, actor, object, unk);
  Poise::UpdateMaxPoise(actor);
  WeaponArt::Manager::UpdateWeaponArt(actor);
}

void Install()
{
  Hook_OnMainUpdate::Install();
  Hook_OnActorUpdate::Install();
  Hook_OnGetAttackStaminaCost::Install();
  Hook_OnGetMeleeDamage::Install();
  Hook_OnWeaponHit::Install();
  Hook_OnPlayIdle::Install();
  Hook_OnPerformAction::Install();
  Hook_OnModActorValue::Install();
  Hook_OnEquipObject::Install();
  Hook_OnUnequipObject::Install();
}
}  // namespace Hooks