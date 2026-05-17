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

void Stamina::WeaponArtStaminaConsume(RE::Actor* actor, std::string payload)
{
  if (!Settings::bUseWeaponArtSystem)
    return;
  if (!actor)
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 2) {
    logger::error("Stamina::WeaponArtStaminaConsume: Invalid payload format: {}", payload);
    return;
  }

  float baseStaminaCost = 1.0f;
  float staminaCostMult = 1.0f;
  try {
    baseStaminaCost = std::stof(split[0]);
    staminaCostMult = std::stof(split[1]);
  } catch (const std::exception& e) {
    logger::error("Stamina::WeaponArtStaminaConsume: Invalid stamina values in payload: {}",
                  payload);
    return;
  }

  if (baseStaminaCost < 0.0f || staminaCostMult < 0.0f)
    return;

  // 不消耗耐力
  if (staminaCostMult == 0.0f)
    return;

  // 使用默认耐力消耗
  if (baseStaminaCost == 0.0f) {
    // 固定使用右手武器的类型来判断战技耐力消耗，因为战技必定由右手武器触发
    auto type       = Weapon::GetActorEquipmentType(actor, false);
    baseStaminaCost = Weapon::GetBaseStaminaConsumption(type);
  }

  float totalStaminaCost = baseStaminaCost * staminaCostMult;
  actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, totalStaminaCost);
}