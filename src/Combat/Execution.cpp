#include "Combat/Execution.h"

#include "Combat/Exhausted.h"
#include "Combat/Weapon.h"
#include "Core/Settings.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using Race       = Execution::Race;
using WeaponType = Weapon::Type;

std::uint16_t operator|(Race r, WeaponType w)
{
  return (static_cast<std::uint16_t>(r) << 8) | static_cast<std::uint16_t>(w);
}

Execution::Execution()
{
  // 从文件加载可用的处决组合

  for (const auto& value : magic_enum::enum_values<WeaponType>()) {
    if (value == WeaponType::None)
      continue;

    availableExcutions.insert(Race::Human | value);
  }
}

void Execution::Update()
{
  // 定期更新可处决Actor列表，移除已不满足条件的Actor
  std::lock_guard lock(mtx);
  if (executableActors.empty())
    return;

  auto now = Utils::GetTime<std::chrono::milliseconds>();
  for (auto it = executableActors.begin(); it != executableActors.end();) {
    if (now - it->second > static_cast<std::uint64_t>(Settings::fExecutableDuration * 1000)) {
      // TODO: 可以在这里添加一些退出处决状态的逻辑，比如通知UI更新等
      if (it->first->IsPlayerRef()) {
        RE::PlayerCharacter::GetSingleton()->SetAIDriven(false);
      } else {
        if (Settings::bDisableAttackWhenExhausted)
          Utils::ActorCanAttack(it->first, !Exhausted::IsActorExhausted(it->first));
        else
          Utils::ActorCanAttack(it->first, true);
      }
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

void Execution::SetExecutable(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExecutionSystem)
    return;

  std::lock_guard lock(mtx);
  executableActors.emplace(actor, Utils::GetTime<std::chrono::milliseconds>());
  // TODO: 可以在这里添加一些进入处决状态的逻辑，比如通知UI更新等
  if (actor->IsPlayerRef()) {
    RE::PlayerCharacter::GetSingleton()->SetAIDriven(true);
  } else {
    Utils::ActorCanAttack(actor, false);
  }
}

void Execution::TryExecute(RE::Actor* aggressor, RE::Actor* victim)
{
  if (!aggressor || !victim || !Settings::bUseExecutionSystem)
    return;

  // 约定只有人类能作为处决者
  if (GetRace(aggressor) != Race::Human)
    return;

  auto race       = GetRace(victim);
  auto weaponType = Weapon::GetActorEquipmentType(aggressor);
  if (!availableExcutions.contains(race | weaponType)) {
    logger::info("Execution::TryExecute: No execution available for Race {} and Weapon {}",
                 magic_enum::enum_name(race), magic_enum::enum_name(weaponType));
    return;
  }

  // 弧度转角度
  constexpr float Deg60  = 60.0f * 3.14159265f / 180.0f;
  constexpr float Deg120 = 120.0f * 3.14159265f / 180.0f;

  // 距离和角度检测
  auto aggressorPos = aggressor->GetPosition();
  auto victimPos    = victim->GetPosition();
  auto direction    = victimPos - aggressorPos;
  auto distance     = direction.Length();
  if (distance > 250.0f)
    return;

  direction           = direction / distance;   // 单位向量，从攻击者指向受害者
  auto aggressorAngle = aggressor->GetAngle();  // 攻击者正前方向（单位向量）
  auto victimAngle    = victim->GetAngle();     // 受害者正前方向（单位向量）

  // 攻击者朝向 与 攻击者→受害者方向 的夹角（弧度）
  auto angleDiff = std::acos(aggressorAngle.Dot(direction));

  // 两个角色朝向之间的夹角（弧度）
  auto angleBetween = std::acos(aggressorAngle.Dot(victimAngle));

  // 正向处决：二者朝向反向，且攻击者正对受害者（60度内）
  bool isFrontExecution = (angleBetween > Deg120)  // 反向：夹角 > 120°
                          && (angleDiff < Deg60);  // 攻击者正对目标

  // 背刺处决：二者朝向同向，且攻击者大致面对受害者（120度内）
  bool isBackExecution = (angleBetween < Deg120)   // 同向：夹角 < 120°
                         && (angleDiff < Deg120);  // 攻击者指向目标方向在120度内

  // 不可能同时满足，但如果发生了，就不执行
  if (isFrontExecution && isBackExecution)
    return;

  if (isFrontExecution) {
    logger::info("Attempting front execution on {} by {}", victim->GetDisplayFullName(),
                 aggressor->GetDisplayFullName());
  }

  if (isBackExecution) {
    logger::info("Attempting back execution on {} by {}", victim->GetDisplayFullName(),
                 aggressor->GetDisplayFullName());
  }
}
