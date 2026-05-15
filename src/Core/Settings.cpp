#include "Core/Settings.h"

#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

#include <filesystem>
#include <fstream>

using json = nlohmann::json;

namespace Settings
{
namespace
{
  bool EnsureSettingsDir()
  {
    std::error_code ec;
    if (!std::filesystem::exists(SettingsDir, ec)) {
      std::filesystem::create_directories(SettingsDir, ec);
    }

    if (ec) {
      logger::warn("Settings: failed to create settings directory: {}", ec.message());
      return false;
    }

    return true;
  }

  template <class T>
  void LoadSetting(const json& section, std::string_view sectionName, std::string_view key,
                   T& value)
  {
    if (auto it = section.find(key); it != section.end()) {
      try {
        value = it->get<T>();
      } catch (const std::exception& e) {
        logger::warn("Settings: failed to parse {}.{}: {}", sectionName, key, e.what());
      }
    }
  }

  bool WriteSettingsFile()
  {
    if (!EnsureSettingsDir())
      return false;

    json root;

    root["Stamina"] = {
        {"UseAttackStaminaSystem", bUseAttackStaminaSystem},
        {"ConsumeStaminaOutCombat", bConsumeStaminaOutCombat},
        {"NormalAttackConsumeStamina", bNormalAttackComsumeStamina},
        {"DisablePlayerAttackWhenStaminaZero", bDisablePlayerAttackWhenStaminaZero},
        {"StaminaRegenMult", fStaminaRegenMult},
        {"StaminaRegenMin", fStaminaRegenMin},
        {"StaminaRegenMax", fStaminaRegenMax},
        {"StaminaRegenDelay", fStaminaRegenDelay},
        {"StaminaRegenMultCombat", fStaminaRegenMultCombat},
        {"StaminaRegenMultBlock", fStaminaRegenMultBlock},
        {"NormalAttackStaminaCostBaseUnarm", fNormalAttackStaminaCostBase_Unarm},
        {"NormalAttackStaminaCostBaseDagger", fNormalAttackStaminaCostBase_Dagger},
        {"NormalAttackStaminaCostBaseSword", fNormalAttackStaminaCostBase_Sword},
        {"NormalAttackStaminaCostBaseAxe", fNormalAttackStaminaCostBase_Axe},
        {"NormalAttackStaminaCostBaseMace", fNormalAttackStaminaCostBase_Mace},
        {"NormalAttackStaminaCostBaseGreatSword", fNormalAttackStaminaCostBase_GreatSword},
        {"NormalAttackStaminaCostBaseGreatAxe", fNormalAttackStaminaCostBase_GreatAxe},
        {"NormalAttackStaminaCostBaseGreatMace", fNormalAttackStaminaCostBase_GreatMace},
        {"NormalAttackStaminaCostBaseFist", fNormalAttackStaminaCostBase_Fist},
        {"NormalAttackStaminaCostPerMass", fNormalAttackStaminaCostPerMass},
        {"PowerAttackStaminaCostMult", fPowerAttackStaminaCostMult},
        {"PowerAttackStaminaCostPerMass", fPowerAttackStaminaCostPerMass}};

    root["Posture"] = {
        {"UsePostureSystem", bUsePostureSystem},
        {"UsePostureHUD", bUsePostureHUD},
        {"MaxPostureBase", fMaxPostureBase},
        {"MaxPostureHealthMult", fMaxPostureHealthMult},
        {"PostureRegenDelay", uPostureRegenDelay},
        {"PostureRegenPercentPerSecond", fPostureRegenPercentPerSecond},
        {"NormalAttackPostureDamageUnarm", fNormalAttackPostureDamage_Unarm},
        {"NormalAttackPostureDamageDagger", fNormalAttackPostureDamage_Dagger},
        {"NormalAttackPostureDamageSword", fNormalAttackPostureDamage_Sword},
        {"NormalAttackPostureDamageAxe", fNormalAttackPostureDamage_Axe},
        {"NormalAttackPostureDamageMace", fNormalAttackPostureDamage_Mace},
        {"NormalAttackPostureDamageGreatSword", fNormalAttackPostureDamage_GreatSword},
        {"NormalAttackPostureDamageGreatAxe", fNormalAttackPostureDamage_GreatAxe},
        {"NormalAttackPostureDamageGreatMace", fNormalAttackPostureDamage_GreatMace},
        {"BashPostureDamageShield", fBashPostureDamage_Shield},
        {"NormalAttackPostureDamageFist", fNormalAttackPostureDamage_Fist},
        {"BashPostureDamageMult", fBashPostureDamageMult},
        {"PowerAttackPostureDamageMult", fPowerAttackPostureDamageMult},
        {"PowerBashPostureDamageMult", fPowerBashPostureDamageMult},
        {"ArmorPostureDamageFactor", fArmorPostureDamageFactor}};

    root["Exhausted"] = {
        {"UseExhaustedSystem", bUseExhaustedSystem},
        {"DisablePlayerAttackWhenExhausted", bDisablePlayerAttackWhenExhausted},
        {"DisableNPCAttackWhenExhausted", bDisableNPCAttackWhenExhausted},
        {"ExitExhaustedOnHit", bExitExhaustedOnHit},
        {"ExhaustedExitPercent", fExhaustedExitPercent},
        {"AttackDamageMultWhenExhausted", fAttackDamageMultWhenExhausted},
        {"AttackPostureDamageMultWhenExhausted", fAttackPostureDamageMultWhenExhausted},
        {"OnHitDamageMultWhenExhausted", fOnHitDamageMultWhenExhausted},
        {"OnHitPostureDamageMultWhenExhausted", fOnHitPostureDamageMultWhenExhausted}};

    root["Block"] = {{"UseBlockSystem", bUseBlockSystem},
                     {"TimedBlockEnabled", bTimedBlockEnabled},
                     {"TimedBlockNeverPostureBreak", bTimedBlockNeverPostureBreak},
                     {"TimedBlockLimit", uTimedBlockLimit},
                     {"BlockMaxStaminaConsumePercent", fBlockMaxStaminaConsumePercent},
                     {"BlockMinStaminaConsume", fBlockMinStaminaConsume},
                     {"BlockStrengthUnarm", fBlockStrength_Unarm},
                     {"BlockStrengthDagger", fBlockStrength_Dagger},
                     {"BlockStrengthSword", fBlockStrength_Sword},
                     {"BlockStrengthAxe", fBlockStrength_Axe},
                     {"BlockStrengthMace", fBlockStrength_Mace},
                     {"BlockStrengthGreatSword", fBlockStrength_GreatSword},
                     {"BlockStrengthGreatAxe", fBlockStrength_GreatAxe},
                     {"BlockStrengthGreatMace", fBlockStrength_GreatMace},
                     {"BlockStrengthShield", fBlockStrength_Shield},
                     {"BlockStrengthFist", fBlockStrength_Fist},
                     {"TimedBlockBlockStrengthMult", fTimedBlockBlockStrengthMult}};

    root["WeaponArt"] = {{"UseWeaponArtSystem", bUseWeaponArtSystem},
                         {"UseWeaponArtHUD", bUseWeaponArtHUD},
                         {"WeaponArtHUDPosX", fWeaponArtHUDPosX},
                         {"WeaponArtHUDPosY", fWeaponArtHUDPosY},
                         {"WeaponArtHUDScale", fWeaponArtHUDScale}};

    root["Execution"] = {{"UseExecutionSystem", bUseExecutionSystem},
                         {"ExitExecutionOnHit", bExitExecutionOnHit},
                         {"ExecutableDuration", uExecutableDuration}};

    try {
      std::ofstream ofs(std::string(SettingsFile), std::ios::out | std::ios::trunc);
      if (!ofs.is_open()) {
        logger::warn("Settings: failed to open settings file for writing: {}", SettingsFile);
        return false;
      }

      ofs << root.dump(4);
      return true;
    } catch (const std::exception& e) {
      logger::warn("Settings: failed to write settings file: {}", e.what());
      return false;
    }
  }
}  // namespace

void SetStaminaRegen(float mult, float upperLimit, float lowerLimit, bool restore = false)
{
  static std::unordered_map<RE::TESRace*, float> originalStaminaRegen;
  if (restore) {
    if (originalStaminaRegen.empty())
      return;
    for (auto& [race, originalRegen] : originalStaminaRegen)
      race->data.staminaRegen = originalRegen;
    return;
  }
  if (originalStaminaRegen.empty()) {
    for (auto& race : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESRace>()) {
      if (race) {
        originalStaminaRegen.try_emplace(race, race->data.staminaRegen);
        float staminaRegen      = race->data.staminaRegen * mult;
        staminaRegen            = fmin(staminaRegen, upperLimit);
        staminaRegen            = fmax(staminaRegen, lowerLimit);
        race->data.staminaRegen = staminaRegen;
      }
    }
  } else {
    for (auto& [race, originalRegen] : originalStaminaRegen) {
      float staminaRegen      = originalRegen * mult;
      staminaRegen            = fmin(staminaRegen, upperLimit);
      staminaRegen            = fmax(staminaRegen, lowerLimit);
      race->data.staminaRegen = staminaRegen;
    }
  }
}

void UpdateGameSettings()
{
  SetStaminaRegen(fStaminaRegenMult, fStaminaRegenMax, fStaminaRegenMin, fStaminaRegenMult == 1.0f);
  Utils::SetGameSettings("fDamagedStaminaRegenDelay", fStaminaRegenDelay);
  Utils::SetGameSettings("fCombatStaminaRegenRateMult", fStaminaRegenMultCombat);
}

void LoadSettings()
{
  if (!EnsureSettingsDir())
    return;

  const auto settingsPath = std::string(SettingsFile);
  if (!std::filesystem::exists(settingsPath)) {
    logger::info("Settings: settings file not found, writing defaults.");
    WriteSettingsFile();
    return;
  }

  json root;
  try {
    std::ifstream ifs(settingsPath);
    if (!ifs.is_open()) {
      logger::warn("Settings: failed to open settings file: {}", settingsPath);
      return;
    }

    root = json::parse(ifs);
  } catch (const std::exception& e) {
    logger::warn("Settings: failed to parse settings file: {}", e.what());
    WriteSettingsFile();
    return;
  }

  if (!root.is_object()) {
    logger::warn("Settings: settings root is not a JSON object.");
    WriteSettingsFile();
    return;
  }

  if (const auto it = root.find("Stamina"); it != root.end() && it->is_object()) {
    const auto& stamina = *it;
    LoadSetting(stamina, "Stamina", "UseAttackStaminaSystem", bUseAttackStaminaSystem);
    LoadSetting(stamina, "Stamina", "ConsumeStaminaOutCombat", bConsumeStaminaOutCombat);
    LoadSetting(stamina, "Stamina", "NormalAttackConsumeStamina", bNormalAttackComsumeStamina);
    LoadSetting(stamina, "Stamina", "DisablePlayerAttackWhenStaminaZero",
                bDisablePlayerAttackWhenStaminaZero);
    LoadSetting(stamina, "Stamina", "StaminaRegenMult", fStaminaRegenMult);
    LoadSetting(stamina, "Stamina", "StaminaRegenMin", fStaminaRegenMin);
    LoadSetting(stamina, "Stamina", "StaminaRegenMax", fStaminaRegenMax);
    LoadSetting(stamina, "Stamina", "StaminaRegenDelay", fStaminaRegenDelay);
    LoadSetting(stamina, "Stamina", "StaminaRegenMultCombat", fStaminaRegenMultCombat);
    LoadSetting(stamina, "Stamina", "StaminaRegenMultBlock", fStaminaRegenMultBlock);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseUnarm",
                fNormalAttackStaminaCostBase_Unarm);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseDagger",
                fNormalAttackStaminaCostBase_Dagger);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseSword",
                fNormalAttackStaminaCostBase_Sword);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseAxe",
                fNormalAttackStaminaCostBase_Axe);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseMace",
                fNormalAttackStaminaCostBase_Mace);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseGreatSword",
                fNormalAttackStaminaCostBase_GreatSword);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseGreatAxe",
                fNormalAttackStaminaCostBase_GreatAxe);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseGreatMace",
                fNormalAttackStaminaCostBase_GreatMace);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostBaseFist",
                fNormalAttackStaminaCostBase_Fist);
    LoadSetting(stamina, "Stamina", "NormalAttackStaminaCostPerMass",
                fNormalAttackStaminaCostPerMass);
    LoadSetting(stamina, "Stamina", "PowerAttackStaminaCostMult", fPowerAttackStaminaCostMult);
    LoadSetting(stamina, "Stamina", "PowerAttackStaminaCostPerMass",
                fPowerAttackStaminaCostPerMass);
  }

  if (const auto it = root.find("Posture"); it != root.end() && it->is_object()) {
    const auto& posture = *it;
    LoadSetting(posture, "Posture", "UsePostureSystem", bUsePostureSystem);
    LoadSetting(posture, "Posture", "UsePostureHUD", bUsePostureHUD);
    LoadSetting(posture, "Posture", "MaxPostureBase", fMaxPostureBase);
    LoadSetting(posture, "Posture", "MaxPostureHealthMult", fMaxPostureHealthMult);
    LoadSetting(posture, "Posture", "PostureRegenDelay", uPostureRegenDelay);
    LoadSetting(posture, "Posture", "PostureRegenPercentPerSecond", fPostureRegenPercentPerSecond);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageUnarm",
                fNormalAttackPostureDamage_Unarm);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageDagger",
                fNormalAttackPostureDamage_Dagger);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageSword",
                fNormalAttackPostureDamage_Sword);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageAxe", fNormalAttackPostureDamage_Axe);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageMace",
                fNormalAttackPostureDamage_Mace);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageGreatSword",
                fNormalAttackPostureDamage_GreatSword);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageGreatAxe",
                fNormalAttackPostureDamage_GreatAxe);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageGreatMace",
                fNormalAttackPostureDamage_GreatMace);
    LoadSetting(posture, "Posture", "BashPostureDamageShield", fBashPostureDamage_Shield);
    LoadSetting(posture, "Posture", "NormalAttackPostureDamageFist",
                fNormalAttackPostureDamage_Fist);
    LoadSetting(posture, "Posture", "BashPostureDamageMult", fBashPostureDamageMult);
    LoadSetting(posture, "Posture", "PowerAttackPostureDamageMult", fPowerAttackPostureDamageMult);
    LoadSetting(posture, "Posture", "PowerBashPostureDamageMult", fPowerBashPostureDamageMult);
    LoadSetting(posture, "Posture", "ArmorPostureDamageFactor", fArmorPostureDamageFactor);
  }

  if (const auto it = root.find("Exhausted"); it != root.end() && it->is_object()) {
    const auto& exhausted = *it;
    LoadSetting(exhausted, "Exhausted", "UseExhaustedSystem", bUseExhaustedSystem);
    LoadSetting(exhausted, "Exhausted", "DisablePlayerAttackWhenExhausted",
                bDisablePlayerAttackWhenExhausted);
    LoadSetting(exhausted, "Exhausted", "DisableNPCAttackWhenExhausted",
                bDisableNPCAttackWhenExhausted);
    LoadSetting(exhausted, "Exhausted", "ExitExhaustedOnHit", bExitExhaustedOnHit);
    LoadSetting(exhausted, "Exhausted", "ExhaustedExitPercent", fExhaustedExitPercent);
    LoadSetting(exhausted, "Exhausted", "AttackDamageMultWhenExhausted",
                fAttackDamageMultWhenExhausted);
    LoadSetting(exhausted, "Exhausted", "AttackPostureDamageMultWhenExhausted",
                fAttackPostureDamageMultWhenExhausted);
    LoadSetting(exhausted, "Exhausted", "OnHitDamageMultWhenExhausted",
                fOnHitDamageMultWhenExhausted);
    LoadSetting(exhausted, "Exhausted", "OnHitPostureDamageMultWhenExhausted",
                fOnHitPostureDamageMultWhenExhausted);
  }

  if (const auto it = root.find("Block"); it != root.end() && it->is_object()) {
    const auto& block = *it;
    LoadSetting(block, "Block", "UseBlockSystem", bUseBlockSystem);
    LoadSetting(block, "Block", "TimedBlockEnabled", bTimedBlockEnabled);
    LoadSetting(block, "Block", "TimedBlockNeverPostureBreak", bTimedBlockNeverPostureBreak);
    LoadSetting(block, "Block", "TimedBlockLimit", uTimedBlockLimit);
    LoadSetting(block, "Block", "BlockMaxStaminaConsumePercent", fBlockMaxStaminaConsumePercent);
    LoadSetting(block, "Block", "BlockMinStaminaConsume", fBlockMinStaminaConsume);
    LoadSetting(block, "Block", "BlockStrengthUnarm", fBlockStrength_Unarm);
    LoadSetting(block, "Block", "BlockStrengthDagger", fBlockStrength_Dagger);
    LoadSetting(block, "Block", "BlockStrengthSword", fBlockStrength_Sword);
    LoadSetting(block, "Block", "BlockStrengthAxe", fBlockStrength_Axe);
    LoadSetting(block, "Block", "BlockStrengthMace", fBlockStrength_Mace);
    LoadSetting(block, "Block", "BlockStrengthGreatSword", fBlockStrength_GreatSword);
    LoadSetting(block, "Block", "BlockStrengthGreatAxe", fBlockStrength_GreatAxe);
    LoadSetting(block, "Block", "BlockStrengthGreatMace", fBlockStrength_GreatMace);
    LoadSetting(block, "Block", "BlockStrengthShield", fBlockStrength_Shield);
    LoadSetting(block, "Block", "BlockStrengthFist", fBlockStrength_Fist);
    LoadSetting(block, "Block", "TimedBlockBlockStrengthMult", fTimedBlockBlockStrengthMult);
  }

  if (const auto it = root.find("WeaponArt"); it != root.end() && it->is_object()) {
    const auto& weaponArt = *it;
    LoadSetting(weaponArt, "WeaponArt", "UseWeaponArtSystem", bUseWeaponArtSystem);
    LoadSetting(weaponArt, "WeaponArt", "UseWeaponArtHUD", bUseWeaponArtHUD);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtHUDPosX", fWeaponArtHUDPosX);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtHUDPosY", fWeaponArtHUDPosY);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtHUDScale", fWeaponArtHUDScale);
  }

  if (const auto it = root.find("Execution"); it != root.end() && it->is_object()) {
    const auto& execution = *it;
    LoadSetting(execution, "Execution", "UseExecutionSystem", bUseExecutionSystem);
    LoadSetting(execution, "Execution", "ExitExecutionOnHit", bExitExecutionOnHit);
    LoadSetting(execution, "Execution", "ExecutableDuration", uExecutableDuration);
  }
}

void SaveSettings()
{
  UpdateGameSettings();
  WriteSettingsFile();
}
}  // namespace Settings