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
  // 战技中不消耗耐力
  // TODO: 新增新的图变量修改战技的耐力消耗倍率
  if (WeaponArt::Manager::IsEnabled(actor))
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

  logger::trace("Stamina::AttackStaminaConsume: type={}, weight={}, cost={}",
                magic_enum::enum_name(type), weaponWeight, staminaCost);
  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
}