#include "Combat/Stamina.h"

#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"

#include "magic_enum/magic_enum.hpp"

using Side = Stamina::Side;

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
    auto weaponObj   = weaponEntry ? weaponEntry->object : nullptr;
    if (weaponObj && weaponObj->IsWeapon()) {
      auto weapon = weaponObj->As<RE::TESObjectWEAP>();
      RE::BGSEntryPoint::HandleEntryPoint(powerEntry, actor, weapon, &finalCost);
    }
  }

  return finalCost;
}

std::tuple<float, float> ParseSide(RE::Actor* actor, Side side)
{
  switch (side) {
  case Side::None:
    return {0.0f, 0.0f};
  case Side::Left: {
    auto type     = Weapon::GetActorEquipmentType(actor, true);
    auto baseCost = Weapon::GetBaseStaminaConsumption(type);
    auto obj      = actor->GetEquippedObject(true);
    return {baseCost, obj ? obj->GetWeight() : 0.0f};
  }
  case Side::Right: {
    auto type     = Weapon::GetActorEquipmentType(actor, false);
    auto baseCost = Weapon::GetBaseStaminaConsumption(type);
    auto obj      = actor->GetEquippedObject(false);
    return {baseCost, obj ? obj->GetWeight() : 0.0f};
  }
  case Side::Creature: {
    auto race     = Race::GetRace(actor);
    auto baseCost = Race::GetBaseStaminaConsumption(race);
    return {baseCost, 0.0f};
  }
  case Side::Auto: {
    auto race = Race::GetRace(actor);
    if (race == Race::Type::Human || race == Race::Type::Draugr || race == Race::Type::Falmer) {
      auto weaponEntry = actor->GetAttackingWeapon();
      auto weaponObj   = weaponEntry ? weaponEntry->object : nullptr;
      auto type        = Weapon::Type::Unarm;
      if (weaponObj && weaponObj->IsWeapon())
        type = Weapon::GetWeaponType(weaponObj->As<RE::TESObjectWEAP>());
      auto baseCost = Weapon::GetBaseStaminaConsumption(type);
      return {baseCost, weaponObj->GetWeight()};
    }
    auto baseCost = Race::GetBaseStaminaConsumption(race);
    return {baseCost, 0.0f};
  }
  }
  return {0.0f, 0.0f};
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

  if (leftType == Weapon::Type::None && rightType == Weapon::Type::None)
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

void Stamina::RimStaminaConsume(RE::Actor* actor, Side side, bool power, float multiplier)
{
  if (!actor || !Settings::bUseAttackStaminaSystem)
    return;

  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto [baseCost, mass] = ParseSide(actor, side);

  if (power)
    baseCost *= Settings::fPowerAttackStaminaCostMult;

  baseCost *= multiplier;

  if (power)
    baseCost += Settings::fPowerAttackStaminaCostPerMass * mass;
  else
    baseCost += Settings::fAttackStaminaCostPerMass * mass;

  baseCost = ApplyPerkEntry(actor, power, baseCost);
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
  if (split.size() != 3) {
    logger::error("Stamina::PayloadParse: Invalid payload format: {}", payload);
    return;
  }

  auto attackType = split[0];
  auto side       = split[1];
  auto multiplier = Utils::toFloat(split[2]);
  if (!multiplier)
    return;
  if (multiplier < 0.0f)
    return;

  bool left  = side == "left";
  bool right = side == "right";

  float baseCost = 0.0f;
  float mass     = 0.0f;
  if (side == "left")
    std::tie(baseCost, mass) = ParseSide(actor, Side::Left);
  else if (side == "right")
    std::tie(baseCost, mass) = ParseSide(actor, Side::Right);
  else if (side == "creature")
    std::tie(baseCost, mass) = ParseSide(actor, Side::Creature);
  else if (side == "auto")
    std::tie(baseCost, mass) = ParseSide(actor, Side::Auto);

  auto power = attackType == "power";

  if (power)
    baseCost *= Settings::fPowerAttackStaminaCostMult;

  baseCost *= multiplier.value();

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