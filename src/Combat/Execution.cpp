#include "Combat/Execution.h"

#include "Combat/Stagger.h"

#include "magic_enum/magic_enum.hpp"

using WeaponType = Weapon::Type;
using RaceType   = Race::Type;

void DisableCollision(RE::Actor* actor)
{
  if (!actor)
    return;

  auto* controller = actor->GetCharController();
  if (!controller)
    return;

  auto zero = _mm_setzero_ps();
  controller->SetLinearVelocityImpl(zero);
  controller->outVelocity.quad     = zero;
  controller->initialVelocity.quad = zero;
  controller->velocityMod.quad     = zero;
  controller->pushDelta.quad       = zero;

  if (auto* rigidBody = controller->GetRigidBody(); rigidBody) {
    float inertiaAndMassInv[4]{};
    _mm_storeu_ps(inertiaAndMassInv, rigidBody->motion.inertiaAndMassInv.quad);
    inertiaAndMassInv[3]                     = 0.0f;
    rigidBody->motion.inertiaAndMassInv.quad = _mm_loadu_ps(inertiaAndMassInv);
    rigidBody->motion.linearVelocity.quad    = _mm_setzero_ps();
    rigidBody->motion.angularVelocity.quad   = _mm_setzero_ps();
  }

  RE::BSAnimationGraphManagerPtr graphMgr;
  if (!actor->GetAnimationGraphManager(graphMgr) || !graphMgr)
    return;

  RE::BSSpinLockGuard locker(graphMgr->GetRuntimeData().updateLock);
  for (auto& graph : graphMgr->graphs) {
    if (!graph)
      continue;

    auto* driver = graph->characterInstance.footIkDriver.get();
    if (driver) {
      auto driverBase     = reinterpret_cast<std::uintptr_t>(driver);
      auto* disableFootIk = reinterpret_cast<bool*>(driverBase + 0x4A);
      *disableFootIk      = true;
    }
  }
}

void EnableCollision(RE::Actor* actor)
{
  if (!actor)
    return;

  RE::BSAnimationGraphManagerPtr graphMgr;
  if (!actor->GetAnimationGraphManager(graphMgr) || !graphMgr)
    return;

  RE::BSSpinLockGuard locker(graphMgr->GetRuntimeData().updateLock);
  for (auto& graph : graphMgr->graphs) {
    if (!graph)
      continue;

    auto* driver = graph->characterInstance.footIkDriver.get();
    if (driver) {
      auto driverBase     = reinterpret_cast<std::uintptr_t>(driver);
      auto* disableFootIk = reinterpret_cast<bool*>(driverBase + 0x4A);
      *disableFootIk      = false;
    }
  }

  auto* controller = actor->GetCharController();
  if (!controller)
    return;

  if (auto* rigidBody = controller->GetRigidBody(); rigidBody) {
    float inertiaAndMassInv[4]{};
    _mm_storeu_ps(inertiaAndMassInv, rigidBody->motion.inertiaAndMassInv.quad);
    inertiaAndMassInv[3]                     = 1.0f;
    rigidBody->motion.inertiaAndMassInv.quad = _mm_loadu_ps(inertiaAndMassInv);
  }
}

Execution::Execution()
{
  // 从文件加载可用的处决组合

  if (!availableExecutions.contains(RaceType::Human))
    availableExecutions[RaceType::Human] = {};

  for (const auto& value : magic_enum::enum_values<WeaponType>()) {
    if (value == WeaponType::None)
      continue;

    availableExecutions[RaceType::Human].push_back(value);
  }
}

void Execution::Update() {}

bool Execution::IsExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return false;

  std::lock_guard<std::mutex> lock(mtx_executable);
  return executableActors.contains(actor);
}

void Execution::EnterExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_executable);
  executableActors.insert(actor);
}

void Execution::ExitExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_executable);
  executableActors.erase(actor);
}

RE::Actor* Execution::FindExecutableTarget(RE::Actor* aggressor)
{
  if (!aggressor || !Settings::bUseExecutionSystem)
    return nullptr;

  std::lock_guard<std::mutex> lock(mtx_executable);
  if (executableActors.empty())
    return nullptr;

  RE::Actor* closestTarget = nullptr;
  float closestDistance    = (std::numeric_limits<float>::max)();
  for (auto* victim : executableActors) {
    float distance = aggressor->GetPosition().GetDistance(victim->GetPosition());
    if (distance > 250.0f)
      continue;
    if (aggressor->GetHeadingAngle(victim->GetPosition(), true) > 60.0f)
      continue;
    if (distance < closestDistance) {
      closestDistance = distance;
      closestTarget   = victim;
    }
  }
  return closestTarget;
}

bool Execution::TryExecute(RE::Actor* aggressor, RE::Actor* victim)
{
  // 处决触发条件：
  // 1. 处决系统开启
  // 2. 攻击者是人类
  // 3. 处决组合满足攻击者的武器类型和目标的种族
  // 4. 距离足够近（250单位）
  // 5. 角度满足要求（面对面或背刺）

  if (!Settings::bUseExecutionSystem)
    return false;

  if (!aggressor || !victim || aggressor->IsDead() || victim->IsDead())
    return false;

  // 约定只有人类能作为处决者
  if (Race::GetRace(aggressor) != RaceType::Human)
    return false;

  auto weaponType = Weapon::GetActorEquipmentType(aggressor);
  auto race       = Race::GetRace(victim);

  std::lock_guard<std::mutex> lock(mtx_executable);
  if (!availableExecutions.contains(race)) {
    logger::info("Execution::TryExecute: No available Idle for using Weapon {} to execute Race {}",
                 magic_enum::enum_name(weaponType), magic_enum::enum_name(race));
    return false;
  }
  auto& availableWeapons = availableExecutions[race];
  if (std::find(availableWeapons.begin(), availableWeapons.end(), weaponType) ==
      availableWeapons.end()) {
    logger::info("Execution::TryExecute: Weapon {} cannot be used to execute Race {}",
                 magic_enum::enum_name(weaponType), magic_enum::enum_name(race));
    return false;
  }

  // 距离检测
  auto aggressorPos = aggressor->GetPosition();
  auto victimPos    = victim->GetPosition();
  auto distance     = aggressorPos.GetDistance(victimPos);
  if (distance > 250.0f)
    return false;

  // 正面 / 背刺判定（攻击者朝向已在 FindExecutableTarget 验证）
  auto victimHeadingToAggressor = victim->GetHeadingAngle(aggressorPos, true);

  bool front = victimHeadingToAggressor < 60.0f;
  bool back  = victimHeadingToAggressor > 120.0f;

  if (front == back)
    return false;

  // 使用Idle无法做到发送killMove
  // 因此这里手动同步位置和角度
  // 并直接发送动画事件来触发动画
  // 因为不使用havok同步
  // 可能存在先后不一致和位移没有锁定的bug

  // 先设置Flag，给OAR缓冲时间来切换动画
  aggressor->SetGraphVariableInt(EXECUTOR_WEAPON, static_cast<std::int32_t>(weaponType));
  aggressor->SetGraphVariableInt(VICTIM_RACE, static_cast<std::int32_t>(race));
  victim->SetGraphVariableInt(EXECUTOR_WEAPON, static_cast<std::int32_t>(weaponType));
  victim->SetGraphVariableInt(VICTIM_RACE, static_cast<std::int32_t>(race));
  Stagger::SetStaggerLevel(victim, Stagger::Level::Execution);
  Stagger::SetStaggerMagnitude(victim, Stagger::Level::Execution);

  // 放置到同一高度
  float maxZ     = (std::max)(aggressorPos.z, victimPos.z);
  aggressorPos.z = maxZ;
  victimPos.z    = maxZ;

  // 根据条件设置角度
  auto heading = aggressor->GetAimHeading();
  heading += aggressor->GetHeadingAngle(victimPos, false);
  aggressor->SetHeading(heading);

  if (back) {
    victim->SetHeading(heading);
  } else {
    float newHeading = 0.0f;
    if (heading >= 0)
      newHeading = heading - 180.0f;
    else
      newHeading = heading + 180.0f;
    victim->SetHeading(newHeading);
  }

  // TODO: 位移，旋转锁定
  DisableCollision(aggressor);
  DisableCollision(victim);

  aggressor->NotifyAnimationGraph("attackStart");
  Stagger::StaggerStart(victim);

  executableActors.erase(victim);

  std::lock_guard<std::mutex> lock_executing(mtx_executing);
  executingActors[victim] = aggressor;
  return true;
}

RE::Actor* Execution::GetExecutingAggressor(RE::Actor* victim)
{
  std::lock_guard<std::mutex> lock(mtx_executing);
  if (executingActors.contains(victim))
    return executingActors[victim];
  return nullptr;
}

bool Execution::IsExecutingVictim(RE::Actor* victim)
{
  std::lock_guard<std::mutex> lock(mtx_executing);
  return executingActors.contains(victim);
}

void Execution::ExecutionEnd(RE::Actor* victim)
{
  if (!victim || !Settings::bUseExecutionSystem)
    return;

  // 处决结束，重置状态
  auto aggressor = GetExecutingAggressor(victim);
  if (aggressor) {
    aggressor->SetGraphVariableInt(EXECUTOR_WEAPON, 0);
    aggressor->SetGraphVariableInt(VICTIM_RACE, 0);
  }

  victim->SetGraphVariableInt(EXECUTOR_WEAPON, 0);
  victim->SetGraphVariableInt(VICTIM_RACE, 0);

  EnableCollision(aggressor);
  EnableCollision(victim);

  std::lock_guard<std::mutex> lock(mtx_executing);
  if (executingActors.contains(victim))
    executingActors.erase(victim);
}

void Execution::AddExecutionStartListener(ExecutionStartCallback callback)
{
  std::lock_guard<std::mutex> lock(mtx_startListeners);
  executionStartListeners.push_back(callback);
}

void Execution::Damage(RE::Actor* victim, const std::string& payload)
{
  if (!victim || !Settings::bUseExecutionSystem)
    return;

  auto aggressor = GetExecutingAggressor(victim);
  if (!aggressor)
    return;

  float damageMult = 1.0f;
  try {
    damageMult = std::stof(payload);
  } catch (const std::exception& e) {
    logger::error("Execution::ApplyExecutionDamage: Invalid damage multiplier in payload: {}",
                  payload);
    return;
  }

  if (damageMult <= 0.0f)
    return;

  float baseDamage = aggressor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kUnarmedDamage);
  if (auto right = aggressor->GetEquippedObject(false); right && right->IsWeapon()) {
    baseDamage = right->As<RE::TESObjectWEAP>()->GetAttackDamage();
    baseDamage += aggressor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMeleeDamage);
  }

  float baseDamageMult = Weapon::GetBaseExecutionMultiplier(aggressor);
  float totalDamage    = baseDamage * baseDamageMult * damageMult;
  logger::info("Execution::ApplyExecutionDamage: Base damage: {} Base multiplier: {} Damage "
               "multiplier: {} Total damage: {}",
               baseDamage, baseDamageMult, damageMult, totalDamage);

  // 处决伤害结算，直接对目标造成真实伤害
  victim->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kHealth, totalDamage);
}

void Execution::SetDistance(RE::Actor* victim, const std::string& payload)
{
  if (!victim || !Settings::bUseExecutionSystem)
    return;

  auto aggressor = GetExecutingAggressor(victim);
  if (!aggressor)
    return;

  float distance = 0.0f;
  try {
    distance = std::stof(payload);
  } catch (const std::exception& e) {
    logger::error("Execution::SetDistance: Invalid distance value in payload: {}", payload);
    return;
  }

  if (distance <= 0.0f)
    return;

  auto aggressorPos = aggressor->GetPosition();
  auto victimPos    = victim->GetPosition();
  auto direction    = (victimPos - aggressorPos);
  direction /= direction.Length();
  victimPos = aggressorPos + direction * distance;
  victim->SetPosition(victimPos, false);
}

void Execution::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload == "end")
    Execution::ExecutionEnd(actor);
  else if (payload.starts_with("damage|"))
    Execution::Damage(actor, payload.substr(7));
  else if (payload.starts_with("setdistance|"))
    Execution::SetDistance(actor, payload.substr(12));
  // 等待拓展
}