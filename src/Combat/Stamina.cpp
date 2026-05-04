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