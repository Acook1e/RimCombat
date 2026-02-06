#include "Posture.h"

#include "Utils.h"
#include <cmath>

std::mutex Posture::postureMutex;

float Posture::InitPosture(RE::Actor* actor)
{
  std::scoped_lock lock(postureMutex);
  float maxPosture = Settings::fMaxPostureBase;
  maxPosture += Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kHealth) * Settings::fMaxPostureHealthMult;
  actor->SetGraphVariableFloat(MaxPostureHealthName.data(), maxPosture);
  actor->SetGraphVariableFloat(CurrentPostureHealthName.data(), maxPosture);
  return maxPosture;
}

float Posture::GetCurrentPosture(RE::Actor* actor)
{
  float result = 0.0f;
  if (actor->GetGraphVariableFloat(CurrentPostureHealthName.data(), result)) {
    return result;
  }
  return 50.0f;
}

float Posture::GetMaxPosture(RE::Actor* actor)
{
  float result = 0.0f;
  if (actor->GetGraphVariableFloat(MaxPostureHealthName.data(), result)) {
    if (result <= 0.0f)
      return InitPosture(actor);
    return result;
  }
  return 100.0f;
}

void Posture::ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData)
{
  auto hitFlags     = hitData.flags;
  auto attackWeapon = hitData.weapon;

  float postureDamage = 0.0f;
  if (!attackWeapon) {
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      postureDamage = Settings::fBashPostureDamageBase;
    } else {
      logger::warn("Posture::ProcessMeleeHit: What are you doing? {}", hitFlags.underlying());
    }
  } else {
    switch (attackWeapon->GetWeaponType()) {
    case RE::WEAPON_TYPE::kHandToHandMelee:
      postureDamage = Settings::fNormalAttackPostureDamage_Fist;
      break;
    case RE::WEAPON_TYPE::kOneHandDagger:
      postureDamage = Settings::fNormalAttackPostureDamage_Dagger;
      break;
    case RE::WEAPON_TYPE::kOneHandSword:
      postureDamage = Settings::fNormalAttackPostureDamage_Sword;
      break;
    case RE::WEAPON_TYPE::kOneHandAxe:
      postureDamage = Settings::fNormalAttackPostureDamage_Axe;
      break;
    case RE::WEAPON_TYPE::kOneHandMace:
      postureDamage = Settings::fNormalAttackPostureDamage_Mace;
      break;
    case RE::WEAPON_TYPE::kTwoHandSword:
      postureDamage = Settings::fNormalAttackPostureDamage_GreatSword;
      break;
    case RE::WEAPON_TYPE::kTwoHandAxe:
      postureDamage = Settings::fNormalAttackPostureDamage_GreatAxe;
      break;
    default:
      logger::warn("Posture::ProcessMeleeHit: Unsupported weapon type: {}", static_cast<int>(attackWeapon->GetWeaponType()));
    }
  }

  // Process Power Attack
  if (hitFlags.any(RE::HitData::Flag::kPowerAttack)) {
    // Process Bash Power Attack
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      postureDamage *= Settings::fPowerBashPostureDamageMult;
    } else {
      postureDamage *= Settings::fPowerAttackPostureDamageMult;
    }
  }

  // Process Block
  if (hitFlags.any(RE::HitData::Flag::kBlocked)) {
    ModPostureValue(aggressor, -postureDamage * Settings::fBlockedPostureDamageToAttacker);
    ModPostureValue(victim, -postureDamage * Settings::fBlockedPostureDamageMult);
  } else {
    ModPostureValue(victim, -postureDamage);
  }
}
void Posture::ModPostureValue(RE::Actor* actor, float value)
{
  if (value == 0.0f)
    return;
  std::scoped_lock lock(postureMutex);
  float currentPosture = GetCurrentPosture(actor);
  float maxPosture     = GetMaxPosture(actor);

  if (value < 0.0f) {
    // Process Posture Damage
    // Armor reduces posture damage taken
    value *= std::expf(-Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kDamageResist) * Settings::fArmorPostureDamageFactor / 1000.0f);
    // Apply Exhausted Multiplier
    if (Settings::bEnableExhausted && IsActorExhausted(actor)) {
      value *= Settings::fExhaustedPostureDamageMult;
      if (Settings::bQuitExhuastedOnHit)
        QuitExhausted(actor);
    }
  } else {
    // Process Posture Recovery
  }

  currentPosture += value;
  if (currentPosture < 0.0f) {
    // TODO
    currentPosture = maxPosture;
  } else if (currentPosture > maxPosture) {
    currentPosture = maxPosture;
  }

  actor->SetGraphVariableFloat(CurrentPostureHealthName.data(), currentPosture);
}

void Posture::InitHUD()
{
  if (!hud)
    hud = reinterpret_cast<TRUEHUD_API::IVTrueHUD4*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V4));
  if (!hud)
    logger::info("InitHUD: TrueHUD API init failed, TrueHUD maybe not installed?");

  if (Settings::bUsePostureHUD) {
    bool enabled = false;
    auto res     = hud->RequestSpecialResourceBarsControl(SKSE::GetPluginHandle());
    switch (res) {
    case TRUEHUD_API::APIResult::OK:
    case TRUEHUD_API::APIResult::AlreadyGiven:
      logger::info("InitHUD: Acquired HUD control.");
      hud->RegisterSpecialResourceFunctions(SKSE::GetPluginHandle(), GetCurrentPosture, GetMaxPosture, true, true);
      enabled = true;
      break;
    case TRUEHUD_API::APIResult::AlreadyTaken:
      logger::info("InitHUD: HUD control already taken by another mod.");
      break;
    }
    if (!enabled)
      Settings::bUsePostureHUD = false;
  }
}

void Posture::ReleaseHUD()
{
  if (!hud)
    return;
  if (hud->ReleaseSpecialResourceBarControl(SKSE::GetPluginHandle()) == TRUEHUD_API::APIResult::OK) {
    Settings::bUsePostureHUD = false;
  }
}

bool Posture::IsActorExhausted(RE::Actor* a_actor)
{
  if (!a_actor)
    return false;
  bool result = false;
  if (a_actor->GetGraphVariableBool("bRimIsExhausted", result))
    return result;
  return false;
}

void Posture::EnterExhausted(RE::Actor* a_actor)
{
  if (!a_actor || !Settings::bEnableExhausted)
    return;
  a_actor->SetGraphVariableBool("bRimIsExhausted", true);
  if (!hud || !Settings::bUsePostureHUD)
    return;
  hud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::FlashColor, 0xd72a2a);
  hud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::BarColor, 0x7d7e7d);
  hud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::PhantomColor, 0xb30d10);
}

void Posture::QuitExhausted(RE::Actor* a_actor)
{
  if (!a_actor || !Settings::bEnableExhausted)
    return;
  a_actor->SetGraphVariableBool("bRimIsExhausted", false);
  if (!hud || !Settings::bUsePostureHUD)
    return;
  hud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::FlashColor);
  hud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::BarColor);
  hud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::PhantomColor);
}
