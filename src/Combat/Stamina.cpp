#include "Combat/Stamina.h"

#include "Combat/Weapon.h"
#include "Combat/WeaponArt.h"
#include "Core/Settings.h"

#include "magic_enum/magic_enum.hpp"

void Stamina::AttackStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm)
{
  if (!Settings::bUseAttackStaminaSystem)
    return;
  if (!actor)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  // 战技动作本身的耐力消耗由自定义事件单独驱动，避免和普通攻击重复扣除。
  if (WeaponArt::Manager::IsPerforming(actor))
    return;

  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto* equipment = actor->GetEquippedObject(leftAttack);

  auto type         = Weapon::GetActorEquipmentType(actor, leftAttack);
  auto weaponWeight = equipment ? equipment->GetWeight() : 0.0f;

  // 避免重复计算
  if (!unarm && type == Weapon::Type::Unarm)
    return;

  float staminaCost = Weapon::GetBaseStaminaConsumption(type);

  if (actor->IsPowerAttacking()) {
    staminaCost *= Settings::fPowerAttackStaminaCostMult;
    staminaCost += Settings::fPowerAttackStaminaCostPerMass * weaponWeight;
  } else {
    staminaCost += Settings::fNormalAttackStaminaCostPerMass * weaponWeight;
  }

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
}

void Stamina::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (!Settings::bUseWeaponArtSystem)
    return;
  if (!actor)
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 3) {
    logger::error("Stamina::PayloadParse: Invalid payload format: {}", payload);
    return;
  }

  std::string attackType = split[0];
  std::string side       = split[1];
  float staminaCostMult  = 1.0f;
  try {
    staminaCostMult = std::stof(split[2]);
  } catch (const std::exception& e) {
    logger::error("Stamina::PayloadParse: Invalid stamina cost multiplier in payload: {}", payload);
    return;
  }

  if (staminaCostMult <= 0.0f)
    return;

  auto type = Weapon::Type::None;
  if (side == "left")
    type = Weapon::GetActorEquipmentType(actor, true);
  else if (side == "right")
    type = Weapon::GetActorEquipmentType(actor, false);
  else if (side == "auto") {
    // 自动检测攻击类型，优先右手
    auto attacking = actor->GetAttackingWeapon();
    auto obj       = attacking ? attacking->object : nullptr;
    if (obj && obj->IsWeapon())
      type = Weapon::GetWeaponType(obj->As<RE::TESObjectWEAP>());
    else
      type = Weapon::GetActorEquipmentType(actor, false);
  } else {
    logger::error("Stamina::PayloadParse: Invalid side in payload: {}", payload);
    return;
  }

  if (type == Weapon::Type::None)
    return;

  float totalStaminaCost = Weapon::GetBaseStaminaConsumption(type) * staminaCostMult;
  if (attackType == "powerattack")
    totalStaminaCost *= Settings::fPowerAttackStaminaCostMult;

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, totalStaminaCost);
}