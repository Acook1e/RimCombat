#pragma once

#include "pch.h"

namespace Handler
{
class AttackHandler
{
public:
  static AttackHandler& GetSingleton()
  {
    static AttackHandler singleton;
    return singleton;
  }

  void ProcessHit(RE::Actor* attacker, RE::Actor* victim, RE::HitData& hitData)
  {
    float damage      = hitData.totalDamage;
    auto hitFlags     = hitData.flags;
    auto attackWeapon = hitData.weapon;

    if (Settings::bUseExhaustionSystem && damage >= 0) {
      if (IsInExhaustion(attacker))
        damage *= Settings::fExhaustionDamageMult;
    }

    // Process Block
    if (hitFlags.any(RE::HitData::Flag::kBlocked)) {
      return;
    }

    // Process Bash
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      if (hitFlags.any(RE::HitData::Flag::kPowerAttack))
        // Power Bash
        return;
      else
        return;
      // Light Bash
    }

    // Process Melee Attack
    if (hitFlags.any(RE::HitData::Flag::kPowerAttack))
      OnPowerAttack(attacker, victim, attackWeapon, damage);
    else
      OnNormalAttack(attacker, victim, attackWeapon, damage);
  }

  void OnNormalAttack(RE::Actor* attacker, RE::Actor* victim, RE::TESObjectWEAP* weapon, float& damage)
  {
    logger::info("AttackHandler: OnNormalAttack called.");
    if (Settings::bNormalAttackComsumeStamina) {
      float weaponMass = 0.0f;
      if (Settings::bConsumeStaminaOutCombat && !attacker->IsInCombat()) {
        if (weapon) {
          weaponMass = weapon->GetWeight();
        }
        float staminaCost =
            Settings::fNormalAttackStaminaCostBase + Settings::fNormalAttackStaminaCostPerMass * weaponMass;
        if (Settings::bNormalAttackComsumeStaminaCheck && damage > 0 &&
            attacker->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) < staminaCost) {
          damage *= Settings::fExhaustionDamageMult;
        }
        attacker->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
      }
    }
  }
  void OnPowerAttack(RE::Actor* attacker, RE::Actor* victim, RE::TESObjectWEAP* weapon, float& damage)
  {
    logger::info("AttackHandler: OnPowerAttack called.");
    if (Settings::bPowerAttackComsumeStaminaTweak) {
      float weaponMass = 0.0f;
      if (Settings::bConsumeStaminaOutCombat && !attacker->IsInCombat()) {
        if (weapon) {
          weaponMass = weapon->GetWeight();
        }
        float staminaCost =
            Settings::fPowerAttackStaminaCostBase + Settings::fPowerAttackStaminaCostPerMass * weaponMass;
        if (Settings::bPowerAttackComsumeStaminaCheck && damage > 0 &&
            attacker->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) < staminaCost) {
          damage *= Settings::fExhaustionDamageMult;
        }
        attacker->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaCost);
      }
    }
  }

  bool IsInExhaustion(RE::Actor* actor)
  {
    if (Settings::exhaustionMark == nullptr)
      return false;
    return actor->HasSpell(Settings::exhaustionMark);
  }

  void EnterExhaustion(RE::Actor* actor)
  {
    if (Settings::exhaustionMark == nullptr)
      return;
    if (!actor->HasSpell(Settings::exhaustionMark))
      actor->AddSpell(Settings::exhaustionMark);
  }

  void QuitExhaustion(RE::Actor* actor)
  {
    if (Settings::exhaustionMark == nullptr)
      return;
    if (actor->HasSpell(Settings::exhaustionMark))
      actor->RemoveSpell(Settings::exhaustionMark);
  }
};
}  // namespace Handler