#include "poiseHandler.h"

namespace Handler::PoiseHandler
{
void RimCombatPoise::ProcessHit(RE::Actor* attacker, RE::Actor* victim, RE::HitData& hitData)
{
  auto hitFlags     = hitData.flags;
  auto attackWeapon = hitData.weapon;

  float poiseDamage = 0.0f;
  if (!attackWeapon) {
    // Bash Behavior
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      poiseDamage = Settings::fBashPoiseDamage;
      if (hitFlags.any(RE::HitData::Flag::kPowerAttack)) {
        poiseDamage *= Settings::fPowerBashPoiseDamageMult;
        DamagePoiseHealth(victim, poiseDamage);
        return;
      } else {
        DamagePoiseHealth(victim, poiseDamage);
        return;
      }
    } else {
      logger::warn("What you are doing? {}", hitFlags.underlying());
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
      break;
    }
  }

  // Process Block
  if (hitFlags.any(RE::HitData::Flag::kBlocked)) {
    // small poise damage to attacker on block
    DamagePoiseHealth(attacker, poiseDamage * Settings::fBlockedPoiseDamageToAttacker);
    DamagePoiseHealth(victim, poiseDamage * Settings::fBlockedPoiseDamageMult);
    return;
  }

  // Process Bash

  // Process Melee Attack
  if (hitFlags.any(RE::HitData::Flag::kPowerAttack)) {
    poiseDamage *= Settings::fPowerAttackPoiseDamageMult;
    DamagePoiseHealth(victim, poiseDamage);
    return;
  } else {
    DamagePoiseHealth(victim, poiseDamage);
    return;
  }
}

float RimCombatPoise::CalculateMaxPoise(RE::Actor* actor)
{
  // For future use, e.g., based on armor, perks, etc.
  return 100.0f;
}

float RimCombatPoise::GetCurrentPoise(RE::Actor* actor)
{
  float result = 10.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(CurrentPoiseHandler_Name, std::ref(result)))
    result = 10.0f;
  mtx.unlock();
  return result;
}

float RimCombatPoise::GetMaxPoise(RE::Actor* actor)
{
  mtx.lock();
  float result = 100.0f;
  if (!actor->GetGraphVariableFloat(MaxPoiseHandler_Name, std::ref(result)))
    result = 100.0f;

  if (result == 0.0f) {
    result = CalculateMaxPoise(actor);
    actor->SetGraphVariableFloat(MaxPoiseHandler_Name, result);
    actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, result);
  }

  mtx.unlock();
  return result;
}

void RimCombatPoise::RestorePoiseHealth(RE::Actor* actor, float a_percent)
{
  if (actor->IsDead())
    return;

  float maxPoise = 100.0f, currentPoise = 0.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(MaxPoiseHandler_Name, std::ref(maxPoise)) ||
      !actor->GetGraphVariableFloat(CurrentPoiseHandler_Name, std::ref(currentPoise))) {
    mtx.unlock();
    return;
  }

  if (maxPoise == 0.0f) {
    maxPoise     = CalculateMaxPoise(actor);
    currentPoise = maxPoise;
    actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, maxPoise);
    actor->SetGraphVariableFloat(MaxPoiseHandler_Name, maxPoise);
  }

  currentPoise += maxPoise * a_percent;
  if (currentPoise > maxPoise)
    currentPoise = maxPoise;

  actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, currentPoise);
  mtx.unlock();
}

void RimCombatPoise::DamagePoiseHealth(RE::Actor* actor, float poiseDamage)
{
  if (poiseDamage <= 0.0f)
    return;

  float currentPoise = 0.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(CurrentPoiseHandler_Name, std::ref(currentPoise))) {
    mtx.unlock();
    return;
  }

  currentPoise -= poiseDamage;
  if (currentPoise < 0.0f) {
    if (Settings::executionMark && !actor->HasSpell(Settings::executionMark)) {
      actor->AddSpell(Settings::executionMark);
      logger::info("RimCombatPoise: Actor {} marked for Execution.", actor->GetDisplayFullName());
      actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, currentPoise);
      std::thread([this, actor]() {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(static_cast<std::int64_t>(Settings::fPoiseBreakStunTime * 1000)));
        this->ResetPoiseHealth(actor);
        if (Settings::executionMark && actor->HasSpell(Settings::executionMark)) {
          actor->RemoveSpell(Settings::executionMark);
          logger::info("RimCombatPoise: Actor {} Execution Mark removed after stun time.", actor->GetDisplayFullName());
        }
      }).detach();
    } else if (Settings::executionMark && actor->HasSpell(Settings::executionMark)) {
      actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, 0.0f);
    } else {
      logger::error("RimCombatPoise: Execution Mark Is Null");
    }
  } else {
    actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, currentPoise);
  }
  mtx.unlock();
}

void RimCombatPoise::ResetPoiseHealth(RE::Actor* actor)
{
  if (actor->IsDead())
    return;

  float maxPoise = 100.0f;
  mtx.lock();
  if (!actor->GetGraphVariableFloat(MaxPoiseHandler_Name, std::ref(maxPoise))) {
    mtx.unlock();
    return;
  }

  actor->SetGraphVariableFloat(CurrentPoiseHandler_Name, maxPoise);
  mtx.unlock();
}

float GetCurrentPoise(RE::Actor* actor)
{
  float result = 10.0f;
  switch (Settings::poiseType) {
  case PoiseType::kMaxsuPoise:
    if (!actor->GetGraphVariableFloat(CurrentMaxsuPoise_Name, result))
      result = 10.0f;
    break;
  case PoiseType::kPoise:
    result = *(float*)(actor + 0x0EC);
    break;
  case PoiseType::kChocolatePoise:
    break;
  case PoiseType::kPoiseHandler:
    result = RimCombatPoise::GetSingleton().GetCurrentPoise(actor);
    break;
  default:
    break;
  }
  return result;
}

float GetMaxPoise(RE::Actor* actor)
{
  float result = 100.0f;
  switch (Settings::poiseType) {
  case PoiseType::kMaxsuPoise:
    break;
  case PoiseType::kPoise:
    break;
  case PoiseType::kChocolatePoise:
    break;
  case PoiseType::kPoiseHandler:
    result = RimCombatPoise::GetSingleton().GetMaxPoise(actor);
    break;
  default:
    break;
  }
  return result;
}
}  // namespace Handler::PoiseHandler