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

std::pair<std::string_view, std::string_view> GetAnimEvent(Race race, bool back)
{
  // 处决者调用的均是成对动画的位置1
  // 受击者调用位置2

  // 除人类外的种族很少具有背刺动画
  if (back) {
    switch (race) {
    case Race::Human:
      // Animations\Paired_1HMKillMoveBackStab.hkx
      return {"pa_KillMove1HMBackStab", "KillMove1HMBackStab"};
    default:
      return {"", ""};
    }
  }

  // 部分种族不具有原版的KillMove
  // 但保留种族的入口以便未来行为图的更新
  switch (race) {
  case Race::Human:
    // Animations\Paired_1HMKillMove.hkx
    return {"pa_KillMove", "KillMove"};
  case Race::Boar:
  case Race::BoarMounted:

  case Race::Chaurus:
  case Race::ChaurusReaper:

  case Race::Dog:
  case Race::Fox:
  case Race::Wolf:

  case Race::Spider:
  case Race::GiantSpider:
  case Race::LargeSpider:

  case Race::Werebear:
  case Race::Werewolf:

  case Race::AshHopper:
  case Race::Bear:
  case Race::ChaurusHunter:
  case Race::Chicken:
  case Race::Cow:
  case Race::Deer:
  case Race::Dragon:
  case Race::DragonPriest:
  case Race::Draugr:
  case Race::DwarvenBallista:
  case Race::DwarvenCenturion:
  case Race::DwarvenSphere:
  case Race::DwarvenSpider:
  case Race::Falmer:
  case Race::FlameAtronach:
  case Race::FrostAtronach:
  case Race::Gargoyle:
  case Race::Giant:
  case Race::Goat:
  case Race::Hagraven:
  case Race::Hare:
  case Race::Horker:
  case Race::Horse:
  case Race::IceWraith:
  case Race::Lurker:
  case Race::Mammoth:
  case Race::Mudcrab:
  case Race::Netch:
  case Race::Riekling:
  case Race::Sabrecat:
  case Race::Seeker:
  case Race::Skeever:
  case Race::Slaughterfish:
  case Race::Spriggan:
  case Race::StormAtronach:
  case Race::Troll:
  case Race::VampireLord:
  case Race::Wisp:
  case Race::Wispmother:
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

    availableExcutions.insert({value | Race::Human, 150.0f});
  }
}

void Execution::Update()
{
  // 定期更新可处决Actor列表，移除已不满足条件的Actor
  std::lock_guard<std::mutex> lock(mtx_executable);
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
  static std::unordered_map<std::string_view, Race> raceMap = {
      {"0_Master.hkx", Race::Human},
      {"WolfBehavior.hkx", Race::Wolf},
      {"DogBehavior.hkx", Race::Dog},
      {"ChickenBehavior.hkx", Race::Chicken},
      {"HareBehavior.hkx", Race::Hare},
      {"AtronachFlameBehavior.hkx", Race::FlameAtronach},
      {"AtronachFrostBehavior.hkx", Race::FrostAtronach},
      {"AtronachStormBehavior.hkx", Race::StormAtronach},
      {"BearBehavior.hkx", Race::Bear},
      {"ChaurusBehavior.hkx", Race::Chaurus},
      {"H-CowBehavior.hkx", Race::Cow},
      {"DeerBehavior.hkx", Race::Deer},
      {"CHaurusFlyerBehavior.hkx", Race::ChaurusHunter},
      {"VampireBruteBehavior.hkx", Race::Gargoyle},
      {"BenthicLurkerBehavior.hkx", Race::Lurker},
      {"BoarBehavior.hkx", Race::Boar},
      {"BCBehavior.hkx", Race::DwarvenBallista},
      {"HMDaedra.hkx", Race::Seeker},
      {"NetchBehavior.hkx", Race::Netch},
      {"RieklingBehavior.hkx", Race::Riekling},
      {"ScribBehavior.hkx", Race::AshHopper},
      {"DragonBehavior.hkx", Race::Dragon},
      {"Dragon_Priest.hkx", Race::DragonPriest},
      {"DraugrBehavior.hkx", Race::Draugr},
      {"SCBehavior.hkx", Race::DwarvenSphere},
      {"DwarvenSpiderBehavior.hkx", Race::DwarvenSpider},
      {"SteamBehavior.hkx", Race::DwarvenCenturion},
      {"FalmerBehavior.hkx", Race::Falmer},
      {"FrostbiteSpiderBehavior.hkx", Race::Spider},
      {"GiantBehavior.hkx", Race::Giant},
      {"GoatBehavior.hkx", Race::Goat},
      {"HavgravenBehavior.hkx", Race::Hagraven},
      {"HorkerBehavior.hkx", Race::Horker},
      {"HorseBehavior.hkx", Race::Horse},
      {"IceWraithBehavior.hkx", Race::IceWraith},
      {"MammothBehavior.hkx", Race::Mammoth},
      {"MudcrabBehavior.hkx", Race::Mudcrab},
      {"SabreCatBehavior.hkx", Race::Sabrecat},
      {"SkeeverBehavior.hkx", Race::Skeever},
      {"SlaughterfishBehavior.hkx", Race::Slaughterfish},
      {"SprigganBehavior.hkx", Race::Spriggan},
      {"TrollBehavior.hkx", Race::Troll},
      {"VampireLord.hkx", Race::VampireLord},
      {"WerewolfBehavior.hkx", Race::Werewolf},
      {"WispBehavior.hkx", Race::Wispmother},
      {"WitchlightBehavior.hkx", Race::Wisp},
  };

  auto behaviorPath =
      actor->GetRace()->rootBehaviorGraphNames[actor->GetActorBase()->IsFemale() ? 1 : 0].data();
  auto behaviorName = std::filesystem::path(behaviorPath).filename().string();

  auto res = Race::None;
  if (auto it = raceMap.find(behaviorName); it != raceMap.end()) {
    res = it->second;
  } else {
    logger::warn("Execution::GetRace: Unknown behavior graph: {}", behaviorName);
    return Race::None;
  }

  if (res == Race::Human)
    return res;

  auto editorId = std::string{actor->GetRace()->GetFormEditorID()};
  switch (res) {
  case Race::Boar:
    static const auto DLC2RieklingMountedKeyword =
        RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x03A159, "Dragonborn.esm");
    if (actor->GetRace()->HasKeyword(DLC2RieklingMountedKeyword))
      res = Race::BoarMounted;
    break;
  case Race::Chaurus:
    if (editorId.find("reaper") != std::string::npos)
      res = Race::ChaurusReaper;
    else
      res = Race::Chaurus;
    break;
  case Race::Spider:
    if (editorId.find("giant") != std::string::npos)
      res = Race::GiantSpider;
    else if (editorId.find("large") != std::string::npos)
      res = Race::LargeSpider;
    break;
  case Race::Wolf:
    if (editorId.find("fox") != std::string::npos)
      res = Race::Fox;
    break;
  case Race::Werewolf:
    if (editorId.find("werebear") != std::string::npos)
      res = Race::Werebear;
    break;
  default:
    break;
  }
  return res;
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

  std::lock_guard<std::mutex> lock(mtx_executable);
  return executableActors.contains(actor);
}

void Execution::EnterExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_executable);
  executableActors.emplace(actor, Utils::GetTime<std::chrono::milliseconds>());
  LockActor(actor);
}

void Execution::ExitExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_executable);
  executableActors.erase(actor);
  UnlockActor(actor);
}

RE::Actor* Execution::FindExecutableTarget(RE::Actor* aggressor)
{
  std::lock_guard<std::mutex> lock(mtx_executable);
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
  if (GetRace(aggressor) != Race::Human)
    return false;

  auto weaponType = Weapon::GetActorEquipmentType(aggressor);
  auto race       = GetRace(victim);
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
    UnlockActor(victim);
    return false;
  }

  std::lock_guard<std::mutex> lock_executing(mtx_executing);
  executingActors[victim] = aggressor;
  executableActors.erase(victim);
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

void Execution::ApplyExecutionDamage(RE::Actor* victim, std::string payload)
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

void Execution::ExecutionEnd(RE::Actor* victim)
{
  if (!victim || !Settings::bUseExecutionSystem)
    return;

  // 处决结束，重置状态
  auto aggressor = GetExecutingAggressor(victim);
  if (aggressor)
    aggressor->SetGraphVariableInt(EXECUTION_FLAG, 0);

  victim->SetGraphVariableInt(EXECUTION_FLAG, 0);

  UnlockActor(victim);

  std::lock_guard<std::mutex> lock(mtx_executing);
  if (executingActors.contains(victim))
    executingActors.erase(victim);
}

void Execution::AddExecutionStartListener(ExecutionStartCallback callback)
{
  std::lock_guard<std::mutex> lock(mtx_startListeners);
  executionStartListeners.push_back(callback);
}