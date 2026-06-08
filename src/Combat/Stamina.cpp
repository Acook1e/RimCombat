#include "Combat/Stamina.h"

#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"

#include "magic_enum/magic_enum.hpp"

float ApplyPerkEntry(RE::Actor* actor, bool powerAttack, float baseCost)
{
  if (!actor)
    return baseCost;

  float finalCost = baseCost;

  auto powerEntry = RE::BGSPerkEntry::EntryPoint::kModPowerAttackStamina;

  if (powerAttack) {
    if (!actor->HasPerkEntries(powerEntry))
      return finalCost;
    auto weaponEntry = actor->GetAttackingWeapon();
    auto weapon      = weaponEntry ? weaponEntry->object->As<RE::TESObjectWEAP>() : nullptr;
    RE::BGSEntryPoint::HandleEntryPoint(powerEntry, actor, weapon, &finalCost);
  }

  return finalCost;
}

Stamina::Stamina()
{
  // 使用序列化重置缓存
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface* serial) {
    {
      std::scoped_lock lock(mtx_rimStamina);
      useRimStaminaActors.clear();
    }
  });
}

void Stamina::SwingStaminaConsume(RE::Actor* actor, RE::TESObjectWEAP* weapon)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  {
    // 使用RimCombat耐力系统的角色不处理Swing的攻击耐力消耗
    std::scoped_lock lock(mtx_rimStamina);
    if (useRimStaminaActors.contains(actor))
      return;
  }

  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto type         = Weapon::GetWeaponType(weapon);
  float staminaCost = Weapon::GetBaseStaminaConsumption(type);

  auto weaponWeight = weapon ? weapon->GetWeight() : 0.0f;
  if (actor->IsPowerAttacking()) {
    staminaCost *= Settings::fPowerAttackStaminaCostMult;
    staminaCost += Settings::fPowerAttackStaminaCostPerMass * weaponWeight;
  } else {
    staminaCost += Settings::fAttackStaminaCostPerMass * weaponWeight;
  }

  staminaCost = ApplyPerkEntry(actor, actor->IsPowerAttacking(), staminaCost);

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
}

void Stamina::CreatureStaminaConsume(RE::Actor* actor, Race::Type raceType)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  {
    // 使用RimCombat耐力系统的角色不处理Creature的攻击耐力消耗
    std::scoped_lock lock(mtx_rimStamina);
    if (useRimStaminaActors.contains(actor))
      return;
  }

  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  float staminaCost = Race::GetBaseStaminaConsumption(raceType);
  if (actor->IsPowerAttacking())
    staminaCost *= Settings::fPowerAttackStaminaCostMult;

  staminaCost = ApplyPerkEntry(actor, actor->IsPowerAttacking(), staminaCost);

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
}

void Stamina::BashStaminaConsume(RE::Actor* actor)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  {
    // 使用RimCombat耐力系统的角色不处理
    std::scoped_lock lock(mtx_rimStamina);
    if (useRimStaminaActors.contains(actor))
      return;
  }

  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto leftType  = Weapon::GetActorEquipmentType(actor, true);
  auto rightType = Weapon::GetActorEquipmentType(actor, false);

  if (leftType == Weapon::Type::None || rightType == Weapon::Type::None)
    return;

  auto baseCost = 0.0f;
  auto mass     = 0.0f;
  if (leftType == Weapon::Type::Shield || leftType == Weapon::Type::Torch) {
    baseCost = Weapon::GetBaseStaminaConsumption(leftType);
    auto obj = actor->GetEquippedObject(true);
    if (obj)
      mass = obj->GetWeight();
  } else {
    baseCost = Weapon::GetBaseStaminaConsumption(rightType);
    auto obj = actor->GetEquippedObject(false);
    if (obj)
      mass = obj->GetWeight();
  }

  if (actor->IsPowerAttacking()) {
    baseCost *= Settings::fPowerBashStaminaCostMult;
    baseCost += Settings::fPowerBashStaminaCostPerMass * mass;
  } else {
    baseCost *= Settings::fBashStaminaCostMult;
    baseCost += Settings::fBashStaminaCostPerMass * mass;
  }

  baseCost = ApplyPerkEntry(actor, actor->IsPowerAttacking(), baseCost);
  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, baseCost);
}

void Stamina::UnarmStaminaConsume(RE::Actor* actor)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  {
    // 使用RimCombat耐力系统的角色不处理
    std::scoped_lock lock(mtx_rimStamina);
    if (useRimStaminaActors.contains(actor))
      return;
  }

  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto baseCost = Weapon::GetBaseStaminaConsumption(Weapon::Type::Unarm);

  if (actor->IsPowerAttacking())
    baseCost *= Settings::fPowerAttackStaminaCostMult;

  baseCost = ApplyPerkEntry(actor, actor->IsPowerAttacking(), baseCost);
  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, baseCost);
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
  auto mass      = 0.0f;
  if (left || right) {
    auto type = Weapon::GetActorEquipmentType(actor, left);
    baseCost  = Weapon::GetBaseStaminaConsumption(type);
    auto obj  = actor->GetEquippedObject(left);
    if (obj)
      mass = obj->GetWeight();
  } else {
    // Auto的情况
    auto attacking = actor->GetAttackingWeapon();
    if (attacking) {
      auto obj = attacking->object;
      if (obj && obj->IsWeapon()) {
        auto type = Weapon::GetWeaponType(obj->As<RE::TESObjectWEAP>());
        if (type != Weapon::Type::None)
          baseCost = Weapon::GetBaseStaminaConsumption(type);
        mass = obj->GetWeight();
      }
    } else {
      // 不存在attackingWeapon，认为是空手攻击
      baseCost = Weapon::GetBaseStaminaConsumption(Weapon::Type::Unarm);
    }
  }

  auto power = attackType == "power";

  if (power)
    baseCost *= Settings::fPowerAttackStaminaCostMult;

  if (subordinate)
    baseCost *= fallbackMultiplier;
  else
    baseCost *= multiplier;

  if (power)
    baseCost += Settings::fPowerAttackStaminaCostPerMass * mass;
  else
    baseCost += Settings::fAttackStaminaCostPerMass * mass;

  baseCost = ApplyPerkEntry(actor, power, baseCost);
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