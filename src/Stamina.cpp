#include "Stamina.h"

float Stamina::AttackStaminaConsume(RE::Actor* a_actor, bool powerAttack, bool consume, bool unarmed)
{
  if (!Settings::bConsumeStaminaOutCombat && !a_actor->IsInCombat()) {
    return 0.0f;
  }
  if (!a_actor)
    return 0.0f;
  if (unarmed) {
    if (a_actor->IsPowerAttacking())
      a_actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, 20.0f);
    else if (!a_actor->IsPowerAttacking() && Settings::bNormalAttackComsumeStamina)
      a_actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, 5.0f);
    return 0.0f;
  }
  bool left       = a_actor->GetEquippedObject(true) ? a_actor->GetEquippedObject(true)->formType == RE::FormType::Weapon : false;
  bool right      = a_actor->GetEquippedObject(false) ? a_actor->GetEquippedObject(false)->formType == RE::FormType::Weapon : false;
  bool leftAttack = false;
  a_actor->GetGraphVariableBool("bLeftHandAttack", leftAttack);
  RE::TESObjectWEAP* weapon;
  if (left && right) {
    if (leftAttack)
      weapon = a_actor->GetEquippedObject(true)->As<RE::TESObjectWEAP>();
    else
      weapon = a_actor->GetEquippedObject(false)->As<RE::TESObjectWEAP>();
  } else if (left) {
    weapon = a_actor->GetEquippedObject(true)->As<RE::TESObjectWEAP>();
  } else if (right) {
    weapon = a_actor->GetEquippedObject(false)->As<RE::TESObjectWEAP>();
  }
  float staminaCost = 0.0f;
  switch (weapon->GetWeaponType()) {
  case RE::WEAPON_TYPE::kOneHandDagger:
    staminaCost = Settings::fNormalAttackStaminaCostBase_Dagger;
    break;
  case RE::WEAPON_TYPE::kOneHandSword:
    staminaCost = Settings::fNormalAttackStaminaCostBase_Sword;
    break;
  case RE::WEAPON_TYPE::kOneHandAxe:
    staminaCost = Settings::fNormalAttackStaminaCostBase_Axe;
    break;
  case RE::WEAPON_TYPE::kOneHandMace:
    staminaCost = Settings::fNormalAttackStaminaCostBase_Mace;
    break;
  case RE::WEAPON_TYPE::kTwoHandSword:
    staminaCost = Settings::fNormalAttackStaminaCostBase_GreatSword;
    break;
  case RE::WEAPON_TYPE::kTwoHandAxe:
    staminaCost = Settings::fNormalAttackStaminaCostBase_GreatAxe;
    break;
  default:
    logger::info("Unknown weapon type {}", static_cast<int>(weapon->GetWeaponType()));
    break;
  }
  if (weapon && weapon->GetWeight() > 0.0f) {
    if (powerAttack && a_actor->IsPowerAttacking()) {
      staminaCost *= Settings::fPowerAttackStaminaCostMult;
      staminaCost += Settings::fPowerAttackStaminaCostPerMass * weapon->GetWeight();
    } else if (!powerAttack && !a_actor->IsPowerAttacking() && Settings::bNormalAttackComsumeStamina)
      staminaCost += Settings::fNormalAttackStaminaCostPerMass * weapon->GetWeight();
  }
  if (consume)
    a_actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
  return staminaCost;
}