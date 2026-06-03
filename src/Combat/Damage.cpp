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
  if (!aggressor || !Settings::bUseDamageSystem)
    return;

  // 不清除，一次攻击可能有多个对象
  std::lock_guard<std::mutex> lock(mtx_damageCache);
  if (auto it = damageCache.find(aggressor); it != damageCache.end())
    damage *= it->second;
}

void Damage::ProcessWeaponDamage(RE::Actor* aggressor, RE::HitData& hitData)
{
  if (!aggressor || !Settings::bUseDamageSystem)
    return;

  bool powerAttack = hitData.flags.any(RE::HitData::Flag::kPowerAttack);
  bool bash        = hitData.flags.any(RE::HitData::Flag::kBash);

  // 将伤害归一化到相对于轻攻击的倍率，方便后续基于倍率进行调整
  if (powerAttack && bash)
    hitData.totalDamage /= 1.5f;
  else if (powerAttack) {
    auto type = Weapon::GetWeaponType(hitData.weapon);
    if (type != Weapon::Type::Unarm)
      hitData.totalDamage /= 2.0f;
  } else if (bash)
    hitData.totalDamage /= 0.5f;

  // 在这里对伤害进行调整，基于归一化后的倍率提供统一的相对1倍倍率的数值调整
  if (powerAttack && bash)
    hitData.totalDamage *= Settings::fDamageMultPowerBash;
  else if (powerAttack)
    hitData.totalDamage *= Settings::fDamageMultPowerAttack;
  else if (bash)
    hitData.totalDamage *= Settings::fDamageMultBash;
}

void Damage::SetMult(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseDamageSystem)
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
  if (!actor || !Settings::bUseDamageSystem)
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