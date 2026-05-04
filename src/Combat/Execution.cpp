#include "Combat/Execution.h"

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
      it = executableActors.erase(it);
      // TODO: 可以在这里添加一些退出处决状态的逻辑，比如通知UI更新等
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

  // 处决逻辑
}
