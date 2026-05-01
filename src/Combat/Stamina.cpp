#include "Combat/Stamina.h"

#include "Combat/Weapon.h"
#include "Core/Settings.h"

void Stamina::AttackStaminaConsume(RE::Actor* actor, bool leftAttack)
{
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;

  if (!actor)
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  RE::TESObjectWEAP* left = nullptr;
  if (auto* leftHand = actor->GetEquippedObject(true);
      leftHand && leftHand->IsWeapon())
    left = leftHand->As<RE::TESObjectWEAP>();

  RE::TESObjectWEAP* right = nullptr;
  if (auto* rightHand = actor->GetEquippedObject(false);
      rightHand && rightHand->IsWeapon())
    right = rightHand->As<RE::TESObjectWEAP>();

  RE::TESObjectWEAP* weapon = nullptr;
  auto type                 = Weapon::Type::Unarm;
  if (leftAttack)
    type = Weapon::GetWeaponType(left);
  else
    type = Weapon::GetWeaponType(right);

  float staminaCost = Weapon::GetBaseStaminaConsumption(type);

  if (actor->IsPowerAttacking())
    staminaCost *= Settings::fPowerAttackStaminaCostMult;

  if (weapon && weapon->GetWeight() > 0.0f) {
    if (actor->IsPowerAttacking())
      staminaCost +=
          Settings::fPowerAttackStaminaCostPerMass * weapon->GetWeight();
    else
      staminaCost +=
          Settings::fNormalAttackStaminaCostPerMass * weapon->GetWeight();
  }

  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina,
                                               staminaCost);
}