#include "Combat/Stamina.h"

#include "Combat/Weapon.h"
#include "Core/Settings.h"

void Stamina::AttackStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm)
{
  if (!Settings::bUseAttackStaminaSystem)
    return;
  if (!actor)
    return;
  if (!Settings::bConsumeStaminaOutCombat && !actor->IsInCombat())
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto type          = Weapon::Type::Unarm;
  float weaponWeight = 0.0f;

  auto GetData = [&]() -> std::pair<Weapon::Type, float> {
    RE::TESObjectWEAP* left = nullptr;
    if (auto* leftHand = actor->GetEquippedObject(true); leftHand && leftHand->IsWeapon())
      left = leftHand->As<RE::TESObjectWEAP>();

    RE::TESObjectWEAP* right = nullptr;
    if (auto* rightHand = actor->GetEquippedObject(false); rightHand && rightHand->IsWeapon())
      right = rightHand->As<RE::TESObjectWEAP>();

    if (leftAttack)
      return {Weapon::GetWeaponType(left), left ? left->GetWeight() : 0.0f};
    else
      return {Weapon::GetWeaponType(right), right ? right->GetWeight() : 0.0f};
  };

  if (!unarm)
    std::tie(type, weaponWeight) = GetData();

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