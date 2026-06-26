#include "Combat/Execution.h"

#include "Combat/Stagger.h"
#include "Core/Settings.h"
#include "Data/Race.h"
#include "Data/Weapon.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using WeaponType = Weapon::Type;
using RaceType   = Race::Type;
using Direction  = Execution::Direction;

std::uint16_t operator|(const WeaponType weapon, const RaceType race)
{
  auto lhs = static_cast<Settings::WeaponEnumType>(weapon);
  auto rhs = static_cast<Settings::RaceEnumType>(race);
  return lhs << 8 | rhs;
}

Direction operator|(Direction lhs, Direction rhs)
{
  auto l = static_cast<std::uint8_t>(lhs);
  auto r = static_cast<std::uint8_t>(rhs);
  return static_cast<Direction>(l | r);
}

bool operator>(Direction lhs, Direction rhs)
{
  auto l = static_cast<std::uint8_t>(lhs);
  auto r = static_cast<std::uint8_t>(rhs);
  return (l & r) == r;
}

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
  const std::string execDir = Settings::SettingsDir + "Execution/";

  std::error_code ec;
  if (!std::filesystem::exists(execDir, ec))
    return;

  for (const auto& entry : std::filesystem::directory_iterator(execDir, ec)) {
    if (ec || !entry.is_regular_file() || entry.path().extension() != ".json")
      continue;

    try {
      std::ifstream file(entry.path());
      nlohmann::json j;
      file >> j;

      for (const auto& [raceName, weapons] : j.items()) {
        auto race = magic_enum::enum_cast<RaceType>(raceName);
        if (!race.has_value()) {
          logger::warn("Execution: unknown race '{}' in {}. Skipping.", raceName,
                       entry.path().string());
          continue;
        }

        if (!weapons.is_object())
          continue;

        for (const auto& [weaponName, directions] : weapons.items()) {
          auto weapon = magic_enum::enum_cast<WeaponType>(weaponName);
          if (!weapon.has_value()) {
            logger::warn("Execution: unknown weapon '{}' for race '{}' in {}. Skipping.",
                         weaponName, raceName, entry.path().string());
            continue;
          }

          if (!directions.is_array())
            continue;

          Direction flag = Direction::None;
          for (const auto& dirStr : directions) {
            auto dir = magic_enum::enum_cast<Direction>(dirStr.get<std::string>());
            if (dir.has_value())
              flag = flag | *dir;
            else
              logger::warn("Execution: unknown direction '{}' for {}|{} in {}. Skipping.",
                           dirStr.get<std::string>(), weaponName, raceName, entry.path().string());
          }

          if (flag != Direction::None)
            availableExecutions[*weapon | *race] = flag;
        }
      }
    } catch (const std::exception& e) {
      logger::error("Execution: failed to parse {}: {}", entry.path().string(), e.what());
    }
  }
}

void Execution::Update() {}

bool Execution::IsExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return false;

  std::shared_lock lock(mtx_executable);
  return executableActors.contains(actor);
}

void Execution::EnterExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::unique_lock lock(mtx_executable);
  executableActors.insert(actor);
}

void Execution::ExitExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::unique_lock lock(mtx_executable);
  executableActors.erase(actor);
}

std::tuple<RE::Actor*, Direction> Execution::FindExecutableTarget(RE::Actor* aggressor)
{
  // 处决触发条件：
  // 1. 处决系统开启
  // 2. 攻击者是人类
  // 3. 处决组合满足攻击者的武器类型和目标的种族
  // 4. 距离足够近（250单位）
  // 5. 角度满足要求（面对面或背刺）

  if (!aggressor || !Settings::bUseExecutionSystem)
    return {nullptr, Direction::None};

  if (aggressor->IsDead() || !aggressor->Is3DLoaded() || aggressor->IsOnMount())
    return {nullptr, Direction::None};

  if (Race::GetRace(aggressor) != RaceType::Human)
    return {nullptr, Direction::None};

  if (IsExecuting(aggressor))
    return {nullptr, Direction::None};

  if (aggressor->IsPlayerRef()) {
    auto camera = RE::PlayerCamera::GetSingleton();
    if (camera && camera->currentState && camera->currentState->id == RE::CameraState::kFirstPerson)
      return {nullptr, Direction::None};
  }

  static const float MinDistance   = 250.0f;
  static const float MaxHeightDiff = 50.0f;

  std::map<float, RE::Actor*> targets;

  {
    std::shared_lock lock(mtx_executable);
    if (executableActors.empty())
      return {nullptr, Direction::None};
    for (auto* victim : executableActors) {
      if (!victim || victim->IsDead() || !victim->Is3DLoaded() || victim->IsOnMount())
        continue;
      if (auto state = victim->AsActorState(); !state || state->IsBleedingOut())
        continue;
      if (victim == aggressor)
        continue;
      if (!aggressor->CheckValidTarget(*victim))
        continue;

      float heightDiff = std::abs(aggressor->GetPosition().z - victim->GetPosition().z);
      if (heightDiff > MaxHeightDiff)
        continue;

      float distance = aggressor->GetPosition().GetDistance(victim->GetPosition());
      if (distance > MinDistance)
        continue;
      if (aggressor->GetHeadingAngle(victim->GetPosition(), true) > 60.0f)
        continue;

      targets.emplace(distance, victim);
    }
  }

  if (targets.empty())
    return {nullptr, Direction::None};

  {
    std::unique_lock lock(mtx_executable);
    for (const auto& [distance, victim] : targets) {
      auto it = executableActors.find(victim);
      if (it == executableActors.end())
        continue;

      auto weapon = Weapon::GetActorEquipmentType(aggressor);
      auto race   = Race::GetRace(victim);

      auto key = weapon | race;
      if (!availableExecutions.contains(key))
        continue;

      auto dir = availableExecutions[key];

      auto victimHeadingToAggressor = victim->GetHeadingAngle(aggressor->GetPosition(), true);
      bool front                    = victimHeadingToAggressor < 60.0f;
      bool back                     = victimHeadingToAggressor > 120.0f;

      if (!front && !back)
        continue;
      if (front == back)
        back = false;

      auto needed = back ? Direction::Back : Direction::Front;
      if (!(dir > needed))
        continue;

      executableActors.erase(it);
      return {victim, needed};
    }
  }

  return {nullptr, Direction::None};
}

bool Execution::Execute(RE::Actor* aggressor, RE::Actor* victim, Direction direction)
{
  if (!Settings::bUseExecutionSystem)
    return false;

  if (!aggressor || !victim || aggressor->IsDead() || victim->IsDead())
    return false;

  auto weapon = Weapon::GetActorEquipmentType(aggressor);
  auto race   = Race::GetRace(victim);

  auto back = (direction == Direction::Back);
  auto dir  = back ? Stagger::Direction::Back : Stagger::Direction::Front;

  // 先设置Flag，给OAR缓冲时间来切换动画
  Stagger::SetStaggerDirection(aggressor, dir);
  Stagger::SetStaggerLevel(aggressor, Stagger::Level::Executor);
  aggressor->SetGraphVariableInt(EXECUTOR_WEAPON, static_cast<std::int32_t>(weapon));
  aggressor->SetGraphVariableInt(VICTIM_RACE, static_cast<std::int32_t>(race));

  Stagger::SetStaggerDirection(victim, dir);
  Stagger::SetStaggerLevel(victim, Stagger::Level::Execution);
  victim->SetGraphVariableInt(EXECUTOR_WEAPON, static_cast<std::int32_t>(weapon));
  victim->SetGraphVariableInt(VICTIM_RACE, static_cast<std::int32_t>(race));

  // 从处决者指向受害者
  auto aggressorPos = aggressor->GetPosition();
  auto victimPos    = victim->GetPosition();
  auto vector       = victimPos - aggressorPos;
  vector /= vector.Length();

  // 受害者处于硬直中不可位移；仅移动处决者到受害者到固定距离
  aggressorPos = victimPos - vector * 50.0f;

  // 统一高度
  auto newZ      = (std::max)(aggressorPos.z, victimPos.z);
  aggressorPos.z = newZ;
  victimPos.z    = newZ;
  aggressor->SetPosition(aggressorPos, true);
  victim->SetPosition(victimPos, true);

  // 定位约定：
  //   FaceToFace — 处决者面对受害者，受害者面对处决者（180°对视）
  //   Backstab   — 处决者面对受害者背部，二者朝向相同
  float heading = std::atan2(vector.x, vector.y);
  aggressor->SetHeading(heading);

  if (!back) {
    constexpr auto PI = std::numbers::pi_v<float>;
    heading += PI;
    if (heading > PI)
      heading -= 2.0f * PI;
  }
  victim->SetHeading(heading);

  Stagger::StaggerStart(aggressor);
  Stagger::StaggerStart(victim);

  {
    std::unique_lock lock(mtx_executing);
    executingActors[aggressor] = victim;
  }
  return true;
}

bool Execution::IsExecuting(RE::Actor* actor)
{
  std::shared_lock lock(mtx_executing);
  if (executingActors.empty())
    return false;

  for (const auto [aggressor, victim] : executingActors) {
    if (actor == aggressor || actor == victim)
      return true;
  }

  return false;
}

RE::Actor* Execution::GetExecutingVictim(RE::Actor* aggressor)
{
  std::shared_lock lock(mtx_executing);
  if (executingActors.contains(aggressor))
    return executingActors[aggressor];
  return nullptr;
}

void Execution::ExecutionEnd(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::unique_lock lock(mtx_executing);
  if (executingActors.contains(actor)) {
    auto victim  = executingActors[actor];
    auto current = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
    if (current < 1.0f)
      victim->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kHealth, 1.0f);
    executingActors.erase(actor);
  }
}

void Execution::AddExecutionStartListener(ExecutionStartCallback callback)
{
  std::lock_guard<std::mutex> lock(mtx_startListeners);
  executionStartListeners.push_back(callback);
}

void Execution::Damage(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  auto victim = GetExecutingVictim(actor);
  if (!victim)
    return;

  auto damageMult = Utils::toFloat(payload);
  if (!damageMult)
    return;

  if (damageMult <= 0.0f)
    return;

  static RE::BGSSoundDescriptorForm* lightSFX = nullptr;
  static RE::BGSSoundDescriptorForm* heavySFX = nullptr;

  if (!lightSFX || !heavySFX) {
    auto data = RE::TESDataHandler::GetSingleton();
    if (data) {
      lightSFX = data->LookupForm<RE::BGSSoundDescriptorForm>(0x809, "RimCombat.esp");
      heavySFX = data->LookupForm<RE::BGSSoundDescriptorForm>(0x80A, "RimCombat.esp");
    }
  }

  if (damageMult < 1.0f)
    Utils::PlaySFX(actor, lightSFX, actor->GetPosition());
  else
    Utils::PlaySFX(actor, heavySFX, actor->GetPosition());

  auto type        = Weapon::GetActorEquipmentType(actor);
  float baseDamage = 0.0f;

  if (type == Weapon::Type::Unarm || type == Weapon::Type::Spell)
    baseDamage = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kUnarmedDamage);
  else if (auto right = actor->GetEquippedObject(false); right && right->IsWeapon()) {
    baseDamage = right->As<RE::TESObjectWEAP>()->GetAttackDamage();
    baseDamage += actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMeleeDamage);
  }

  if (baseDamage == 0.0f)
    return;

  float baseDamageMult = Weapon::GetBaseExecutionMultiplier(actor);
  float totalDamage    = baseDamage * baseDamageMult * damageMult.value();
  // logger::info("Execution::ApplyExecutionDamage: Base damage: {} Base multiplier: {} Damage "
  //              "multiplier: {} Total damage: {}",
  //              baseDamage, baseDamageMult, damageMult.value(), totalDamage);

  // 处决过程免死
  auto current = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kHealth);
  if (current < totalDamage)
    totalDamage = current - 0.5;
  else if (current < 1.0f)
    totalDamage = 0;

  // 处决伤害结算，直接对目标造成真实伤害
  victim->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kHealth, totalDamage);
}

void Execution::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload == "end")
    Execution::ExecutionEnd(actor);
  else if (payload.starts_with("damage|"))
    Execution::Damage(actor, payload.substr(7));
  // 等待拓展
}