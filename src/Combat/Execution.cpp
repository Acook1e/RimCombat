#include "Combat/Execution.h"

#include "Combat/Exhausted.h"
#include "Combat/Weapon.h"
#include "Core/Settings.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using Race       = Execution::Race;
using WeaponType = Weapon::Type;

std::uint16_t operator|(WeaponType w, Race r)
{
  return (static_cast<std::uint16_t>(w) << 8) | static_cast<std::uint16_t>(r);
}

Execution::Execution()
{
  // 从文件加载可用的处决组合

  auto KillMoveShortA = RE::TESForm::LookupByID<RE::TESIdleForm>(0x6440D);

  for (const auto& value : magic_enum::enum_values<WeaponType>()) {
    if (value == WeaponType::None)
      continue;

    availableExcutions.emplace((value | Race::Human), KillMoveShortA);
  }
}

void Execution::Update()
{
  // 定期更新可处决Actor列表，移除已不满足条件的Actor
  std::lock_guard<std::mutex> lock(mtx);
  if (executableActors.empty())
    return;

  auto now = Utils::GetTime<std::chrono::milliseconds>();
  for (auto it = executableActors.begin(); it != executableActors.end();) {
    if (now - it->second > Settings::uExecutableDuration) {
      UnlockActor(it->first);
      it = executableActors.erase(it);
    } else {
      ++it;
    }
  }
}

Race Execution::GetRace(RE::Actor* actor)
{
  if (!actor)
    return Race::None;

  return Race::Human;
}

void Execution::LockActor(RE::Actor* actor)
{
  if (!actor)
    return;

  // 首先强制打断当前动作
  actor->InterruptCast(false);
  actor->GetActorRuntimeData().currentProcess->StopCurrentIdle(actor, true);

  actor->SetGraphVariableBool(EXECUTABLE, true);

  if (actor->IsPlayerRef()) {
    // 对于玩家启用AI驱动，但因为不存在真正的AI，所以相当于禁用玩家控制权
    RE::PlayerCharacter::GetSingleton()->SetAIDriven(true);
  } else {
    // 对于NPC禁用移动
    actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kUnconcious;
  }

  // 发送默认姿势事件，由OAR条件播放对应的动画
  Utils::AddTask([actor]() {
    actor->NotifyAnimationGraph("IdleDefaultStart");
  });
}

void Execution::UnlockActor(RE::Actor* actor)
{
  if (!actor)
    return;

  actor->SetGraphVariableBool(EXECUTABLE, false);

  if (actor->IsPlayerRef()) {
    // 对于玩家禁用AI驱动，恢复玩家控制权
    RE::PlayerCharacter::GetSingleton()->SetAIDriven(false);
  } else {
    // 对于NPC重新启用移动
    actor->AsActorState()->actorState1.lifeState = RE::ACTOR_LIFE_STATE::kAlive;
  }
}

bool Execution::IsExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return false;

  std::lock_guard<std::mutex> lock(mtx);
  return executableActors.contains(actor);
}

void Execution::EnterExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx);
  executableActors.emplace(actor, Utils::GetTime<std::chrono::milliseconds>());
  LockActor(actor);
}

void Execution::ExitExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx);
  executableActors.erase(actor);
  UnlockActor(actor);
}

RE::Actor* Execution::FindExecutableTarget(RE::Actor* aggressor)
{
  std::lock_guard<std::mutex> lock(mtx);
  if (executableActors.empty())
    return nullptr;

  RE::Actor* closestTarget = nullptr;
  float closestDistance    = (std::numeric_limits<float>::max)();
  for (auto& [actor, timestamp] : executableActors) {
    float distance = aggressor->GetPosition().GetDistance(actor->GetPosition());
    if (distance > 250.0f)
      continue;
    if (distance < closestDistance) {
      closestDistance = distance;
      closestTarget   = actor;
    }
  }
  return closestTarget;
}

bool Execution::TryExecute(RE::Actor* aggressor, RE::Actor* victim)
{
  if (!aggressor || !victim || !Settings::bUseExecutionSystem)
    return false;

  // 约定只有人类能作为处决者
  if (GetRace(aggressor) != Race::Human)
    return false;

  auto weaponType = Weapon::GetActorEquipmentType(aggressor);
  auto race       = GetRace(victim);
  if (!availableExcutions.contains(weaponType | race)) {
    logger::info("Execution::TryExecute: No available Idle for using Weapon {} to execute Race {}",
                 magic_enum::enum_name(weaponType), magic_enum::enum_name(race));
    return false;
  }

  constexpr float Deg60  = 60.0f;
  constexpr float Deg120 = 120.0f;

  // 距离和角度检测
  auto aggressorPos = aggressor->GetPosition();
  auto victimPos    = victim->GetPosition();
  auto distance     = aggressorPos.GetDistance(victimPos);
  if (distance > 250.0f)
    return false;

  // GetHeadingAngle返回的是-180 ~ 180度
  // 正负表示左右偏，绝对值表示偏转角度
  auto aggressorHeadingToVictim = aggressor->GetHeadingAngle(victimPos, true);
  auto victimHeadingToAggressor = victim->GetHeadingAngle(aggressorPos, true);

  // 面对面处决：双方都基本正对对方。
  bool isFrontExecution = aggressorHeadingToVictim < Deg60 && victimHeadingToAggressor < Deg60;

  // 背刺处决：攻击者面对目标，但目标基本背对攻击者。
  bool isBackExecution = aggressorHeadingToVictim < Deg60 && victimHeadingToAggressor > Deg120;

  // 同时不满足或同时满足都不执行
  if (isFrontExecution == isBackExecution)
    return false;

  if (isFrontExecution) {
    logger::info("Attempting front execution on {} by {}", victim->GetDisplayFullName(),
                 aggressor->GetDisplayFullName());
    aggressor->SetGraphVariableBool(BACKSTAB, false);
  }

  if (isBackExecution) {
    logger::info("Attempting back execution on {} by {}", victim->GetDisplayFullName(),
                 aggressor->GetDisplayFullName());
    aggressor->SetGraphVariableBool(BACKSTAB, true);
  }

  auto* idle = availableExcutions[weaponType | race];

  aggressor->GetActorRuntimeData().currentProcess->PlayIdle(aggressor, idle, victim);
  executableActors.erase(victim);
  return true;
}