#include "poiseHandler.h"

#include "executionHandler.h"
#include "setting.h"

namespace Handler::Poise
{
void RimPoise::ProcessMeleeHit(RE::Actor* attacker, RE::Actor* victim, RE::HitData& hitData)
{
  auto hitFlags     = hitData.flags;
  auto attackWeapon = hitData.weapon;

  float poiseDamage = 0.0f;
  if (!attackWeapon) {
    // Bash Behavior
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      poiseDamage = Settings::fBashPoiseDamageBase;
    } else {
      logger::warn("Handler::Poise::ProcessMeleeHit: What are you doing? {}", hitFlags.underlying());
    }
  } else {
    switch (attackWeapon->GetWeaponType()) {
    case RE::WEAPON_TYPE::kHandToHandMelee:
      poiseDamage = Settings::fNormalAttackPoiseDamage_Fist;
      break;
    case RE::WEAPON_TYPE::kOneHandDagger:
      poiseDamage = Settings::fNormalAttackPoiseDamage_Dagger;
      break;
    case RE::WEAPON_TYPE::kOneHandSword:
      poiseDamage = Settings::fNormalAttackPoiseDamage_Sword;
      break;
    case RE::WEAPON_TYPE::kOneHandAxe:
      poiseDamage = Settings::fNormalAttackPoiseDamage_Axe;
      break;
    case RE::WEAPON_TYPE::kOneHandMace:
      poiseDamage = Settings::fNormalAttackPoiseDamage_Mace;
      break;
    case RE::WEAPON_TYPE::kTwoHandSword:
      poiseDamage = Settings::fNormalAttackPoiseDamage_GreatSword;
      break;
    case RE::WEAPON_TYPE::kTwoHandAxe:
      poiseDamage = Settings::fNormalAttackPoiseDamage_GreatAxe;
      break;
    default:
      logger::warn("Handler::Poise::ProcessMeleeHit: Unsupported weapon type: {}",
                   static_cast<int>(attackWeapon->GetWeaponType()));
      break;
    }
  }

  // Process Power Attack
  if (hitFlags.any(RE::HitData::Flag::kPowerAttack)) {
    // Process Bash Power Attack
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      poiseDamage *= Settings::fPowerBashPoiseDamageMult;
    } else {
      poiseDamage *= Settings::fPowerAttackPoiseDamageMult;
    }
  }

  // Process Block
  if (hitFlags.any(RE::HitData::Flag::kBlocked)) {
    DamagePoiseHealth(attacker, poiseDamage * Settings::fBlockedPoiseDamageToAttacker);
    DamagePoiseHealth(victim, poiseDamage * Settings::fBlockedPoiseDamageMult);
  } else {
    DamagePoiseHealth(victim, poiseDamage);
  }
  return;
}

float RimPoise::CalculateMaxPoise(RE::Actor* actor)
{
  // For future use, e.g., based on armor, perks, etc.
  float basePoise = Settings::fActorMaxPoiseBase;
  return basePoise;
}

float RimPoise::GetCurrentPoise(RE::Actor* actor)
{
  float result = 10.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(CurrentPoiseStr, result))
    result = 10.0f;
  mtx.unlock();
  return result;
}

float RimPoise::GetMaxPoise(RE::Actor* actor)
{
  mtx.lock();
  float result = 100.0f;
  if (!actor->GetGraphVariableFloat(MaxPoiseStr, result))
    result = 100.0f;

  if (result == 0.0f) {
    result = CalculateMaxPoise(actor);
    actor->SetGraphVariableFloat(MaxPoiseStr, result);
    actor->SetGraphVariableFloat(CurrentPoiseStr, result);
  }

  mtx.unlock();
  return result;
}

void RimPoise::RestorePoiseHealth(RE::Actor* actor, float a_percent)
{
  if (actor->IsDead())
    return;

  float maxPoise = 100.0f, currentPoise = 0.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(MaxPoiseStr, maxPoise) ||
      !actor->GetGraphVariableFloat(CurrentPoiseStr, currentPoise)) {
    mtx.unlock();
    return;
  }

  if (maxPoise == 0.0f) {
    maxPoise     = CalculateMaxPoise(actor);
    currentPoise = maxPoise;
    actor->SetGraphVariableFloat(CurrentPoiseStr, maxPoise);
    actor->SetGraphVariableFloat(MaxPoiseStr, maxPoise);
  }

  currentPoise += maxPoise * a_percent;
  if (currentPoise > maxPoise)
    currentPoise = maxPoise;

  actor->SetGraphVariableFloat(CurrentPoiseStr, currentPoise);
  mtx.unlock();
}

void RimPoise::DamagePoiseHealth(RE::Actor* actor, float poiseDamage)
{
  if (poiseDamage <= 0.0f)
    return;

  float currentPoise = 0.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(CurrentPoiseStr, currentPoise)) {
    mtx.unlock();
    return;
  }

  currentPoise -= poiseDamage;
  if (currentPoise <= 0.0f) {
    if (!Handler::Execution::GetSingleton().IsVictim(actor)) {
      actor->SetGraphVariableFloat(CurrentPoiseStr, currentPoise);
      Handler::Execution::GetSingleton().EnterExecutionState(actor);
    } else
      actor->SetGraphVariableFloat(CurrentPoiseStr, 0.0f);
  } else {
    actor->SetGraphVariableFloat(CurrentPoiseStr, currentPoise);
  }
  mtx.unlock();
}

void RimPoise::ResetPoiseHealth(RE::Actor* actor)
{
  if (actor->IsDead())
    return;

  float maxPoise = 100.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(MaxPoiseStr, maxPoise)) {
    mtx.unlock();
    return;
  }

  actor->SetGraphVariableFloat(CurrentPoiseStr, maxPoise);
  mtx.unlock();
}
}  // namespace Handler::Poise