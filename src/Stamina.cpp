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
  float weaponMass = 0.0f;
  bool left        = a_actor->GetEquippedObject(true) ? a_actor->GetEquippedObject(true)->formType == RE::FormType::Weapon : false;
  bool right       = a_actor->GetEquippedObject(false) ? a_actor->GetEquippedObject(false)->formType == RE::FormType::Weapon : false;
  bool leftAttack  = false;
  a_actor->GetGraphVariableBool("bLeftHandAttack", leftAttack);
  if (left && right) {
    if (leftAttack)
      weaponMass = a_actor->GetEquippedObject(true)->GetWeight();
    else
      weaponMass = a_actor->GetEquippedObject(false)->GetWeight();
  } else if (left) {
    weaponMass = a_actor->GetEquippedObject(true)->GetWeight();
  } else if (right) {
    weaponMass = a_actor->GetEquippedObject(false)->GetWeight();
  }
  float staminaCost = 0.0f;
  if (weaponMass > 0.0f) {
    if (powerAttack && a_actor->IsPowerAttacking())
      staminaCost = Settings::fPowerAttackStaminaCostBase + Settings::fPowerAttackStaminaCostPerMass * weaponMass;
    else if (!powerAttack && !a_actor->IsPowerAttacking() && Settings::bNormalAttackComsumeStamina)
      staminaCost = Settings::fNormalAttackStaminaCostBase + Settings::fNormalAttackStaminaCostPerMass * weaponMass;
  }
  if (consume)
    a_actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
  return staminaCost;
}