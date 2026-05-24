#include "Combat/Stamina.h"

#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Race.h"
#include "Data/Weapon.h"

#include "magic_enum/magic_enum.hpp"

float GetBaseStaminaConsumption(RE::Actor* actor, bool leftHand)
{
  auto type = Weapon::GetActorEquipmentType(actor, leftHand);
  if (type != Weapon::Type::None)
    return Weapon::GetBaseStaminaConsumption(type);

  auto race = Race::GetRace(actor);
  return Race::GetBaseStaminaConsumption(race);
}

float ApplyPerkEntry(RE::Actor* actor, bool powerAttack, float baseCost)
{
  if (!actor)
    return baseCost;

  float finalCost = baseCost;

  auto powerEntry = RE::BGSPerkEntry::EntryPoint::kModPowerAttackStamina;

  // 无法判断乘法与加法的区别

  return finalCost;
}

Stamina::Stamina()
{
  // 使用序列化重置缓存
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface* serial) {
    {
      std::scoped_lock lock(mtx_precision);
      usePrecisionStaminaActors.clear();
    }
    {
      std::scoped_lock lock(mtx_rimStamina);
      useRimStaminaActors.clear();
    }
  });
}

void Stamina::SwingStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  {
    // 使用Precision系统的角色不处理Swing的攻击耐力消耗
    std::scoped_lock lock(mtx_precision);
    if (usePrecisionStaminaActors.contains(actor))
      return;
  }
  {
    // 使用RimCombat耐力系统的角色不处理Swing的攻击耐力消耗
    std::scoped_lock lock(mtx_rimStamina);
    if (useRimStaminaActors.contains(actor))
      return;
  }

  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  // 避免重复计算
  auto type = Weapon::GetActorEquipmentType(actor, leftAttack);
  if (!unarm && type == Weapon::Type::Unarm)
    return;

  float staminaCost = GetBaseStaminaConsumption(actor, leftAttack);

  auto* equipment   = actor->GetEquippedObject(leftAttack);
  auto weaponWeight = equipment ? equipment->GetWeight() : 0.0f;
  if (actor->IsPowerAttacking()) {
    staminaCost *= Settings::fPowerAttackStaminaCostMult;
    staminaCost += Settings::fPowerAttackStaminaCostPerMass * weaponWeight;
  } else {
    staminaCost += Settings::fNormalAttackStaminaCostPerMass * weaponWeight;
  }

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
}

void Stamina::PrecisionStart(RE::Actor* actor)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  std::scoped_lock lock(mtx_precision);
  usePrecisionStaminaActors.insert(actor);
}

void Stamina::PrecisionEnd(RE::Actor* actor)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  std::scoped_lock lock(mtx_precision);
  usePrecisionStaminaActors.erase(actor);
}

void Stamina::CollisionStaminaConsume(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  {
    // 只有在使用RimCombat耐力系统时忽略基于Collision事件的攻击耐力消耗
    std::scoped_lock lock(mtx_rimStamina);
    if (useRimStaminaActors.contains(actor))
      return;
  }

  bool right = payload.find("weapon") != std::string::npos;
  bool left  = payload.find("shield") != std::string::npos;

  if (!right && !left) {
    logger::error("Stamina::CollisionStaminaConsume: Invalid payload for collision_add: {}",
                  payload);
    return;
  }

  auto* equipment   = actor->GetEquippedObject(left);
  auto weaponWeight = equipment ? equipment->GetWeight() : 0.0f;

  float staminaCost = GetBaseStaminaConsumption(actor, left);
  if (actor->IsPowerAttacking()) {
    staminaCost *= Settings::fPowerAttackStaminaCostMult;
    staminaCost += Settings::fPowerAttackStaminaCostPerMass * weaponWeight;
  } else
    staminaCost += Settings::fNormalAttackStaminaCostPerMass * weaponWeight;

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
}

void Stamina::Start(RE::Actor* actor)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  std::scoped_lock lock(mtx_rimStamina);
  useRimStaminaActors.insert(actor);
}

void Stamina::End(RE::Actor* actor)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  std::scoped_lock lock(mtx_rimStamina);
  useRimStaminaActors.erase(actor);
}

void Stamina::Consume(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 3 && split.size() != 4) {
    logger::error("Stamina::PayloadParse: Invalid payload format: {}", payload);
    return;
  }

  auto attackType = split[0];
  auto side       = split[1];

  float multiplier         = 1.0f;
  float fallbackMultiplier = 1.0f;

  bool subordinate =
      WeaponArt::Manager::GetPerform(actor) == WeaponArt::Manager::Perform::Subordinate;

  multiplier = Utils::toFloat(split[2]);
  // 只有在Subordinate中才处理FallbackMultiplier
  if (subordinate && split.size() == 4)
    fallbackMultiplier = Utils::toFloat(split[3]);

  if (multiplier < 0.0f || fallbackMultiplier < 0.0f)
    return;

  bool left  = side == "left";
  bool right = side == "right";

  float baseCost = 0.0f;
  if (left || right)
    baseCost = GetBaseStaminaConsumption(actor, left);
  else {
    // Auto的情况
    auto attacking = actor->GetAttackingWeapon();
    auto obj       = attacking ? attacking->object : nullptr;
    if (obj && obj->IsWeapon()) {
      auto type = Weapon::GetWeaponType(obj->As<RE::TESObjectWEAP>());
      if (type == Weapon::Type::None)
        baseCost = GetBaseStaminaConsumption(actor, false);
      else
        baseCost = Weapon::GetBaseStaminaConsumption(type);
    } else if (!obj)
      baseCost = GetBaseStaminaConsumption(actor, false);
  }

  if (attackType == "power")
    baseCost *= Settings::fPowerAttackStaminaCostMult;

  if (subordinate)
    baseCost *= fallbackMultiplier;
  else
    baseCost *= multiplier;

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, baseCost);
}

void Stamina::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  if (payload == "start")
    Start(actor);
  else if (payload == "end")
    End(actor);
  else if (payload.starts_with("consume|"))
    Consume(actor, payload.substr(8));
}