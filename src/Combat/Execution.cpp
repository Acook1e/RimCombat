#include "Combat/Execution.h"

#include "Combat/Exhausted.h"
#include "Combat/Stagger.h"
#include "Core/Settings.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using WeaponType = Weapon::Type;
using RaceType   = Race::Type;

void DisableCollision(RE::Actor* actor)
{
  if (!actor)
    return;

  auto* controller = actor->GetCharController();
  if (!controller)
    return;

  controller->pitchAngle          = 0.0f;
  controller->rollAngle           = 0.0f;
  controller->calculatePitchTimer = 55.0f;

  auto zero = _mm_setzero_ps();
  controller->SetLinearVelocityImpl(zero);

  controller->outVelocity.quad      = zero;
  controller->initialVelocity.quad  = zero;
  controller->velocityMod.quad      = zero;
  controller->pushDelta.quad        = zero;
  controller->fakeSupportStart.quad = zero;
  controller->supportNorm.quad      = _mm_setr_ps(0.0f, 0.0f, 1.0f, 0.0f);

  controller->flags.set(RE::CHARACTER_FLAGS::kNoGravityOnGround, RE::CHARACTER_FLAGS::kNoSim,
                        RE::CHARACTER_FLAGS::kSupport);

  controller->flags.reset(RE::CHARACTER_FLAGS::kCheckSupport,
                          RE::CHARACTER_FLAGS::kHasPotentialSupportManifold,
                          RE::CHARACTER_FLAGS::kStuckQuad, RE::CHARACTER_FLAGS::kOnStairs,
                          RE::CHARACTER_FLAGS::kTryStep);

  controller->context.currentState = RE::hkpCharacterStateType::kSwimming;

  // DisableRigidBodyPhysics
  controller->flags.set(RE::CHARACTER_FLAGS::kNotPushablePermanent);
  controller->flags.reset(RE::CHARACTER_FLAGS::kPossiblePathObstacle);

  controller->surfaceInfo.surfaceVelocity.quad = _mm_setzero_ps();

  if (auto* rigidBody = controller->GetRigidBody(); rigidBody) {
    float inertiaAndMassInv[4]{};
    _mm_storeu_ps(inertiaAndMassInv, rigidBody->motion.inertiaAndMassInv.quad);
    inertiaAndMassInv[3]                     = 0.0f;
    rigidBody->motion.inertiaAndMassInv.quad = _mm_loadu_ps(inertiaAndMassInv);
    rigidBody->motion.gravityFactor          = 0.0f;
    // clear linear and angular velocity
    rigidBody->motion.linearVelocity.quad  = _mm_setzero_ps();
    rigidBody->motion.angularVelocity.quad = _mm_setzero_ps();
  }

  // Disable Foot IK
  RE::BSAnimationGraphManagerPtr graphMgr;
  if (!actor->GetAnimationGraphManager(graphMgr) || !graphMgr)
    return;

  RE::BSSpinLockGuard locker(graphMgr->GetRuntimeData().updateLock);
  for (auto& graph : graphMgr->graphs) {
    if (!graph)
      continue;

    auto* driver = graph->characterInstance.footIkDriver.get();
    if (driver) {
      auto driverBase             = reinterpret_cast<std::uintptr_t>(driver);
      auto* disableFootIk         = reinterpret_cast<bool*>(driverBase + 0x4A);
      auto* alignedGroundRotation = reinterpret_cast<RE::hkQuaternion*>(driverBase + 0x30);

      *disableFootIk             = true;
      alignedGroundRotation->vec = {0.0f, 0.0f, 0.0f, 1.0f};
    }
  }
}

void EnableCollision(RE::Actor* actor)
{
  if (!actor)
    return;

  // Enable Foot IK
  RE::BSAnimationGraphManagerPtr graphMgr;
  if (!actor->GetAnimationGraphManager(graphMgr) || !graphMgr)
    return;

  RE::BSSpinLockGuard locker(graphMgr->GetRuntimeData().updateLock);
  for (auto& graph : graphMgr->graphs) {
    if (!graph)
      continue;

    auto* driver = graph->characterInstance.footIkDriver.get();
    if (driver) {
      auto driverBase             = reinterpret_cast<std::uintptr_t>(driver);
      auto* disableFootIk         = reinterpret_cast<bool*>(driverBase + 0x4A);
      auto* alignedGroundRotation = reinterpret_cast<RE::hkQuaternion*>(driverBase + 0x30);

      *disableFootIk             = false;
      alignedGroundRotation->vec = {0.0f, 0.0f, 0.0f, 1.0f};
    }
  }

  // EnableRigidBodyPhysics
  auto* controller = actor->GetCharController();
  if (!controller)
    return;

  controller->flags.reset(RE::CHARACTER_FLAGS::kNoGravityOnGround, RE::CHARACTER_FLAGS::kNoSim,
                          RE::CHARACTER_FLAGS::kNotPushablePermanent);

  controller->flags.set(RE::CHARACTER_FLAGS::kSupport, RE::CHARACTER_FLAGS::kCheckSupport);

  controller->context.currentState = RE::hkpCharacterStateType::kOnGround;

  if (auto* rigidBody = controller->GetRigidBody(); rigidBody) {
    float inertiaAndMassInv[4]{};
    _mm_storeu_ps(inertiaAndMassInv, rigidBody->motion.inertiaAndMassInv.quad);
    inertiaAndMassInv[3]                     = 1.0f;
    rigidBody->motion.inertiaAndMassInv.quad = _mm_loadu_ps(inertiaAndMassInv);
    rigidBody->motion.gravityFactor          = 1.0f;
  }
}

Execution::Execution()
{
  // 从文件加载可用的处决组合

  if (!availableExcutions.contains(RaceType::Human))
    availableExcutions[RaceType::Human] = {};

  for (const auto& value : magic_enum::enum_values<WeaponType>()) {
    if (value == WeaponType::None)
      continue;

    availableExcutions[RaceType::Human].push_back(value);
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
  if (!availableExcutions.contains(race)) {
    logger::info("Execution::TryExecute: No available Idle for using Weapon {} to execute Race {}",
                 magic_enum::enum_name(weaponType), magic_enum::enum_name(race));
    return false;
  }
  auto& availableWeapons = availableExcutions[race];
  if (std::find(availableWeapons.begin(), availableWeapons.end(), weaponType) ==
      availableWeapons.end()) {
    logger::info("Execution::TryExecute: Weapon {} cannot be used to execute Race {}",
                 magic_enum::enum_name(weaponType), magic_enum::enum_name(race));
    return false;
  }

  constexpr float MaxAggressorAngleDiff = 60.0f;

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

  // 如果攻击者与目标的相对角度过大，则无法触发处决
  if (aggressorHeadingToVictim > MaxAggressorAngleDiff)
    return false;

  // 正面处决条件：攻击者和目标相互面向
  bool front = victimHeadingToAggressor < 60.0f;

  // 背刺处决条件：攻击者面向目标，且目标背对攻击者
  bool back = victimHeadingToAggressor > 120.0f;

  // 同时满足或者同时不满足正面和背刺则视为不满足处决条件
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
  Stagger::StaggerStart(aggressor);

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