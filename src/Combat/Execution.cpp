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

std::string_view GetVictimAnimEvent(Race race, bool back)
{
  // 调用的均是成对动画中的位置2

  if (back) {
    switch (race) {
    case Race::Human:
      // KillMove1HMBackStab -> Animations\Paired_1HMKillMoveBackStab.hkx
      return "KillMove1HMBackStab";
    default:
      return "";
    }
  }

  switch (race) {
  case Race::Human:
    // KillMove -> Animations\Paired_1HMKillMove.hkx
    return "KillMove";
  default:
    return "";
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

  std::lock_guard<std::mutex> lock(mtx_executable);
  if (!availableExcutions.contains(weaponType | race)) {
    logger::info("Execution::TryExecute: No available Idle for using Weapon {} to execute Race {}",
                 magic_enum::enum_name(weaponType), magic_enum::enum_name(race));
    return false;
  }

  float initDistance     = availableExcutions[weaponType | race];
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

  // 判断是否存在对应的动画事件
  auto victimAnimEvent = GetVictimAnimEvent(race, isBackExecution);
  if (victimAnimEvent.empty()) {
    logger::info("Execution::TryExecute: Executing Race {} in the {} is not supported yet",
                 magic_enum::enum_name(race), isBackExecution ? "back" : "front");
    return false;
  }

  // 使用Idle无法做到发送killMove
  // 因此这里手动同步位置和角度
  // 并直接发送动画事件来触发动画
  // 因为不使用havok同步
  // 可能存在先后不一致和位移没有锁定的bug

  // 放置到同一高度
  float maxZ     = (std::max)(aggressorPos.z, victimPos.z);
  aggressorPos.z = maxZ;
  victimPos.z    = maxZ;

  // 设置统一的距离
  // TODO: 可能需要根据不同的动画包调整这个距离
  // 但目前默认 150.0f
  auto direction = (victimPos - aggressorPos);
  direction /= direction.Length();
  auto squr150 = std::sqrt(initDistance);
  victimPos    = aggressorPos + direction * squr150;
  aggressor->SetPosition(aggressorPos, false);
  victim->SetPosition(victimPos, false);

  // 根据条件设置角度
  auto heading = aggressor->GetAimHeading();
  heading += aggressor->GetHeadingAngle(victimPos, false);
  aggressor->SetHeading(heading);
  if (isFrontExecution) {
    float newHeading = 0.0f;
    if (heading >= 0)
      newHeading = heading - 180.0f;
    else
      newHeading = heading + 180.0f;
    victim->SetHeading(newHeading);
  } else if (isBackExecution) {
    victim->SetHeading(heading);
  }

  // 正向时调用的动画
  // pa_KillMove -> Animations\Paired_1HMKillMove.hkx
  if (isFrontExecution)
    aggressor->NotifyAnimationGraph("pa_KillMove");

  // 背向时调用的动画
  // pa_KillMove1HMBackStab -> Animations\Paired_1HMKillMoveBackStab.hkx
  if (isBackExecution)
    aggressor->NotifyAnimationGraph("pa_KillMove1HMBackStab");

  // 处决者调用的动画是确认的
  // 受击者的动画事件由GetVictimAnimEvent根据种族和背刺与否来确认
  victim->NotifyAnimationGraph(victimAnimEvent);

  std::lock_guard<std::mutex> lock_executing(mtx_executing);
  executingActors[aggressor] = victim;
  executableActors.erase(victim);

  UnlockActor(victim);
  return true;
}

RE::Actor* Execution::GetExecutionVictim(RE::Actor* aggressor)
{
  std::lock_guard<std::mutex> lock(mtx_executing);
  if (executingActors.contains(aggressor))
    return executingActors[aggressor];
  return nullptr;
}

void Execution::ExecutionEnd(RE::Actor* aggressor)
{
  if (!aggressor)
    return;

  std::lock_guard<std::mutex> lock(mtx_executing);
  if (executingActors.contains(aggressor))
    executingActors.erase(aggressor);
}

void Execution::AddExecutionStartListener(ExecutionStartCallback callback)
{
  std::lock_guard<std::mutex> lock(mtx_startListeners);
  executionStartListeners.push_back(callback);
}