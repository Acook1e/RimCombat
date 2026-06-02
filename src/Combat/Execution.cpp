#include "Combat/Execution.h"

#include "Combat/Exhausted.h"
#include "Combat/Stagger.h"
#include "Core/Settings.h"
#include "Data/Race.h"
#include "Data/Weapon.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using WeaponType = Weapon::Type;
using RaceType   = Race::Type;

std::uint16_t operator|(WeaponType w, RaceType r)
{
  return (static_cast<std::uint16_t>(w) << 8) | static_cast<std::uint16_t>(r);
}

std::pair<std::string_view, std::string_view> GetAnimEvent(RaceType race, bool back)
{
  // 处决者调用的均是成对动画的位置1
  // 受击者调用位置2

  // 除人类外的种族很少具有背刺动画
  if (back) {
    switch (race) {
    case RaceType::Human:
      // Animations\Paired_1HMKillMoveBackStab.hkx
      return {"pa_KillMove1HMBackStab", "KillMove1HMBackStab"};
    default:
      return {"", ""};
    }
  }

  // 部分种族不具有原版的KillMove
  // 但保留种族的入口以便未来行为图的更新
  switch (race) {
  case RaceType::Human:
    // Animations\Paired_1HMKillMove.hkx
    return {"pa_KillMove", "KillMove"};
  case RaceType::Boar:
  case RaceType::BoarMounted:

  case RaceType::Chaurus:
  case RaceType::ChaurusReaper:

  case RaceType::Dog:
  case RaceType::Fox:
  case RaceType::Wolf:

  case RaceType::Spider:
  case RaceType::GiantSpider:
  case RaceType::LargeSpider:

  case RaceType::Werebear:
  case RaceType::Werewolf:

  case RaceType::AshHopper:
  case RaceType::Bear:
  case RaceType::ChaurusHunter:
  case RaceType::Chicken:
  case RaceType::Cow:
  case RaceType::Deer:
  case RaceType::Dragon:
  case RaceType::DragonPriest:
  case RaceType::Draugr:
  case RaceType::DwarvenBallista:
  case RaceType::DwarvenCenturion:
  case RaceType::DwarvenSphere:
  case RaceType::DwarvenSpider:
  case RaceType::Falmer:
  case RaceType::FlameAtronach:
  case RaceType::FrostAtronach:
  case RaceType::Gargoyle:
  case RaceType::Giant:
  case RaceType::Goat:
  case RaceType::Hagraven:
  case RaceType::Hare:
  case RaceType::Horker:
  case RaceType::Horse:
  case RaceType::IceWraith:
  case RaceType::Lurker:
  case RaceType::Mammoth:
  case RaceType::Mudcrab:
  case RaceType::Netch:
  case RaceType::Riekling:
  case RaceType::Sabrecat:
  case RaceType::Seeker:
  case RaceType::Skeever:
  case RaceType::Slaughterfish:
  case RaceType::Spriggan:
  case RaceType::StormAtronach:
  case RaceType::Troll:
  case RaceType::VampireLord:
  case RaceType::Wisp:
  case RaceType::Wispmother:
  default:
    return {"", ""};
  }
}

Execution::Execution()
{
  // 从文件加载可用的处决组合

  // 因为调用的是原版的动画，为了保证原版的动画的可用性
  // 约定0表示原版的动画
  // 非0状态OAR会根据这个值来判断播放哪个动画
  for (const auto& value : magic_enum::enum_values<WeaponType>()) {
    if (value == WeaponType::None)
      continue;

    availableExcutions.insert({value | RaceType::Human, 150.0f});
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
  auto flag       = weaponType | race;

  std::lock_guard<std::mutex> lock(mtx_executable);
  if (!availableExcutions.contains(flag)) {
    logger::info("Execution::TryExecute: No available Idle for using Weapon {} to execute Race {}",
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

  // 判断是否存在对应的动画事件
  auto [aggressorAnimEvent, victimAnimEvent] = GetAnimEvent(race, back);
  if (aggressorAnimEvent.empty() || victimAnimEvent.empty()) {
    logger::info("Execution::TryExecute: Executing Race {} in the {} is not supported yet",
                 magic_enum::enum_name(race), back ? "back" : "front");
    return false;
  }

  // 使用Idle无法做到发送killMove
  // 因此这里手动同步位置和角度
  // 并直接发送动画事件来触发动画
  // 因为不使用havok同步
  // 可能存在先后不一致和位移没有锁定的bug

  // 先设置Flag，给OAR缓冲时间来切换动画
  aggressor->SetGraphVariableInt(EXECUTION_FLAG, flag);
  victim->SetGraphVariableInt(EXECUTION_FLAG, flag);

  // 放置到同一高度
  float maxZ     = (std::max)(aggressorPos.z, victimPos.z);
  aggressorPos.z = maxZ;
  victimPos.z    = maxZ;

  // 根据不同的动画调整距离
  auto direction = (victimPos - aggressorPos);
  direction /= direction.Length();
  float initDistance = availableExcutions[flag];
  victimPos          = aggressorPos + direction * initDistance;
  aggressor->SetPosition(aggressorPos, false);
  victim->SetPosition(victimPos, false);

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

  if (!aggressor->NotifyAnimationGraph(aggressorAnimEvent) ||
      !victim->NotifyAnimationGraph(victimAnimEvent)) {
    logger::warn("Execution::TryExecute: Failed to send animation event. Aggressor: {}, Victim: {}",
                 aggressor->GetDisplayFullName(), victim->GetDisplayFullName());
    return false;
  }

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
  if (aggressor)
    aggressor->SetGraphVariableInt(EXECUTION_FLAG, 0);

  victim->SetGraphVariableInt(EXECUTION_FLAG, 0);

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

void Execution::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload == "end")
    Execution::ExecutionEnd(actor);
  else if (payload.starts_with("damage|"))
    Execution::Damage(actor, payload.substr(7));
  // 等待拓展
}