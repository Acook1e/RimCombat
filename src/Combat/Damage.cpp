#include "Combat/Damage.h"

#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Utils.h"

Damage::Damage()
{
  // 使用序列化系统重置缓存
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    std::lock_guard<std::mutex> lock(mtx_damageCache);
    damageCache.clear();
  });
}

void Damage::ProcessDamage(RE::Actor* aggressor, float& damage)
{
  if (!aggressor)
    return;

  // 不清除，一次攻击可能有多个对象
  std::lock_guard<std::mutex> lock(mtx_damageCache);
  if (auto it = damageCache.find(aggressor); it != damageCache.end())
    damage *= it->second;
}

void Damage::SetMult(RE::Actor* actor, const std::string& payload)
{
  if (!actor)
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 1 && split.size() != 2)
    return;

  float multiplier         = Utils::toFloat(split[0]);
  float fallbackMultiplier = split.size() == 2 ? Utils::toFloat(split[1]) : multiplier;

  bool subordinate =
      WeaponArt::Manager::GetPerform(actor) == WeaponArt::Manager::Perform::Subordinate;
  if (multiplier < 0.0f || fallbackMultiplier < 0.0f)
    return;

  std::lock_guard<std::mutex> lock(mtx_damageCache);
  damageCache[actor] = subordinate ? fallbackMultiplier : multiplier;
}

void Damage::End(RE::Actor* actor)
{
  if (!actor)
    return;

  std::lock_guard<std::mutex> lock(mtx_damageCache);
  damageCache.erase(actor);
}

void Damage::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload.starts_with("setmult|"))
    SetMult(actor, payload.substr(8));
  else if (payload == "end")
    End(actor);
}