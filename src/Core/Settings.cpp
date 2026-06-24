#include "Core/Settings.h"

#include "Data/Race.h"
#include "Data/Weapon.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace Settings
{
namespace
{
  using WeaponType = Weapon::Type;
  using RaceType   = Race::Type;

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

  constexpr WeaponEnumType ToWeaponKey(WeaponType type)
  {
    return static_cast<WeaponEnumType>(type);
  }

  constexpr RaceEnumType ToRaceKey(RaceType type)
  {
    return static_cast<RaceEnumType>(type);
  }

  void ClearSettingMaps()
  {
    baseStaminaCostMap.clear();
    baseCreatureStaminaMap.clear();
    basePostureMap.clear();
    basePostureDamageMap.clear();
    baseCreaturePostureDamage.clear();
    basePoiseMap.clear();
    basePoiseDamageMap.clear();
    baseCreaturePoiseDamage.clear();
    blockStrengthMap.clear();
    executionDamageMultMap.clear();
  }

  bool OverwriteSettingsFromDefault()
  {
    std::error_code ec;
    if (!std::filesystem::exists(SettingsDefaultFile, ec)) {
      logger::warn("Settings: default settings file not found: {}", SettingsDefaultFile);
      return false;
    }

    std::filesystem::copy_file(SettingsDefaultFile, SettingsFile,
                               std::filesystem::copy_options::overwrite_existing, ec);
    if (ec) {
      logger::warn("Settings: failed to overwrite settings file from default: {}", ec.message());
      return false;
    }

    return true;
  }

  void SaveWeaponFloatMap(json& section, std::string_view subsectionName,
                          const std::unordered_map<WeaponEnumType, float>& map)
  {
    auto& subsection = section[std::string(subsectionName)];
    subsection       = json::object();
    for (auto type : magic_enum::enum_values<WeaponType>()) {
      if (type == WeaponType::None)
        continue;

      auto name = magic_enum::enum_name(type);
      if (name.empty())
        continue;

      if (auto it = map.find(ToWeaponKey(type)); it != map.end())
        subsection[std::string(name)] = it->second;
    }
  }

  void SaveRaceFloatMap(json& section, std::string_view subsectionName,
                        const std::unordered_map<RaceEnumType, float>& map)
  {
    auto& subsection = section[std::string(subsectionName)];
    subsection       = json::object();
    for (auto type : magic_enum::enum_values<RaceType>()) {
      if (type == RaceType::None)
        continue;

      auto name = magic_enum::enum_name(type);
      if (name.empty())
        continue;

      if (auto it = map.find(ToRaceKey(type)); it != map.end())
        subsection[std::string(name)] = it->second;
    }
  }

  void LoadWeaponFloatMap(const json& section, std::string_view sectionName,
                          std::string_view subsectionName,
                          std::unordered_map<WeaponEnumType, float>& map)
  {
    if (const auto it = section.find(subsectionName); it != section.end()) {
      if (!it->is_object()) {
        logger::warn("Settings: {}.{} is not a JSON object.", sectionName, subsectionName);
        return;
      }

      const auto& subsection   = *it;
      const auto qualifiedName = std::string(sectionName) + "." + std::string(subsectionName);
      for (auto type : magic_enum::enum_values<WeaponType>()) {
        if (type == WeaponType::None)
          continue;

        auto name = magic_enum::enum_name(type);
        if (name.empty())
          continue;

        if (auto valueIt = subsection.find(name); valueIt != subsection.end()) {
          try {
            map.insert_or_assign(ToWeaponKey(type), valueIt->get<float>());
          } catch (const std::exception& e) {
            logger::warn("Settings: failed to parse {}.{}: {}", qualifiedName, name, e.what());
          }
        }
      }
    }
  }

  void LoadRaceFloatMap(const json& section, std::string_view sectionName,
                        std::string_view subsectionName,
                        std::unordered_map<RaceEnumType, float>& map)
  {
    if (const auto it = section.find(subsectionName); it != section.end()) {
      if (!it->is_object()) {
        logger::warn("Settings: {}.{} is not a JSON object.", sectionName, subsectionName);
        return;
      }

      const auto& subsection   = *it;
      const auto qualifiedName = std::string(sectionName) + "." + std::string(subsectionName);
      for (auto type : magic_enum::enum_values<RaceType>()) {
        if (type == RaceType::None)
          continue;

        auto name = magic_enum::enum_name(type);
        if (name.empty())
          continue;

        if (auto valueIt = subsection.find(name); valueIt != subsection.end()) {
          try {
            map.insert_or_assign(ToRaceKey(type), valueIt->get<float>());
          } catch (const std::exception& e) {
            logger::warn("Settings: failed to parse {}.{}: {}", qualifiedName, name, e.what());
          }
        }
      }
    }
  }

  bool WriteSettingsFile()
  {
    if (!EnsureSettingsDir())
      return false;

    json root;

    root["Damage"] = {{"UseDamageSystem", bUseDamageSystem},
                      {"DamageMultPowerAttack", fDamageMultPowerAttack},
                      {"DamageMultBash", fDamageMultBash},
                      {"DamageMultPowerBash", fDamageMultPowerBash}};

    json stamina = {{"UseAttackStaminaSystem", bUseAttackStaminaSystem},
                    {"ConsumeStaminaOutCombat", bConsumeStaminaOutCombat},
                    {"DisableAttackWhenStaminaZero", bDisableAttackWhenStaminaZero},
                    {"StaminaRegenMult", fStaminaRegenMult},
                    {"StaminaRegenMin", fStaminaRegenMin},
                    {"StaminaRegenMax", fStaminaRegenMax},
                    {"StaminaRegenDelay", fStaminaRegenDelay},
                    {"StaminaRegenMultCombat", fStaminaRegenMultCombat},
                    {"StaminaRegenMultBlock", fStaminaRegenMultBlock},
                    {"AttackStaminaCostPerMass", fAttackStaminaCostPerMass},
                    {"PowerAttackStaminaCostMult", fPowerAttackStaminaCostMult},
                    {"PowerAttackStaminaCostPerMass", fPowerAttackStaminaCostPerMass},
                    {"BashStaminaCostMult", fBashStaminaCostMult},
                    {"BashStaminaCostPerMass", fBashStaminaCostPerMass},
                    {"PowerBashStaminaCostMult", fPowerBashStaminaCostMult},
                    {"PowerBashStaminaCostPerMass", fPowerBashStaminaCostPerMass}};
    SaveWeaponFloatMap(stamina, "BaseStaminaCost", baseStaminaCostMap);
    SaveRaceFloatMap(stamina, "BaseCreatureStamina", baseCreatureStaminaMap);
    root["Stamina"] = std::move(stamina);

    json posture = {{"UsePostureSystem", bUsePostureSystem},
                    {"UsePostureHUD", bUsePostureHUD},
                    {"DisablePlayerPostureBreak", bDisablePlayerPostureBreak},
                    {"MaxPostureHealthMult", fMaxPostureHealthMult},
                    {"MaxPostureStaminaMult", fMaxPostureStaminaMult},
                    {"PostureRegenDelay", uPostureRegenDelay},
                    {"PostureRegenPercentPerSecond", fPostureRegenPercentPerSecond},
                    {"BashPostureDamageMult", fBashPostureDamageMult},
                    {"PowerAttackPostureDamageMult", fPowerAttackPostureDamageMult},
                    {"PowerBashPostureDamageMult", fPowerBashPostureDamageMult},
                    {"ArmorPostureDamageFactor", fArmorPostureDamageFactor}};
    SaveRaceFloatMap(posture, "BasePosture", basePostureMap);
    SaveWeaponFloatMap(posture, "BasePostureDamage", basePostureDamageMap);
    SaveRaceFloatMap(posture, "BaseCreaturePostureDamage", baseCreaturePostureDamage);
    root["Posture"] = std::move(posture);

    json poise = {{"UsePoiseSystem", bUsePoiseSystem},
                  {"PoiseStaminaMult", fPoiseStaminaMult},
                  {"PoiseMassMult", fPoiseMassMult},
                  {"PoiseRegenDelay", uPoiseRegenDelay},
                  {"PoiseRegenPercentPerSecond", fPoiseRegenPercentPerSecond},
                  {"StaggerCompensationPercent", fStaggerCompensationPercent},
                  {"StaggerLevelSmall", fStaggerLevelSmall},
                  {"StaggerLevelMedium", fStaggerLevelMedium},
                  {"StaggerLevelLarge", fStaggerLevelLarge},
                  {"LightArmorHeadMaxPoiseBonus", fLightArmorHeadMaxPoiseBonus},
                  {"LightArmorBodyMaxPoiseBonus", fLightArmorBodyMaxPoiseBonus},
                  {"LightArmorHandMaxPoiseBonus", fLightArmorHandMaxPoiseBonus},
                  {"LightArmorFeetMaxPoiseBonus", fLightArmorFeetMaxPoiseBonus},
                  {"HeavyArmorHeadMaxPoiseBonus", fHeavyArmorHeadMaxPoiseBonus},
                  {"HeavyArmorBodyMaxPoiseBonus", fHeavyArmorBodyMaxPoiseBonus},
                  {"HeavyArmorHandMaxPoiseBonus", fHeavyArmorHandMaxPoiseBonus},
                  {"HeavyArmorFeetMaxPoiseBonus", fHeavyArmorFeetMaxPoiseBonus},
                  {"BashPoiseDamageMult", fBashPoiseDamageMult},
                  {"PowerAttackPoiseDamageMult", fPowerAttackPoiseDamageMult},
                  {"PowerBashPoiseDamageMult", fPowerBashPoiseDamageMult},
                  {"LevelDiffAggressorMultPerLevel", fLevelDiffAggressorMultPerLevel},
                  {"VictimAttackingPoiseBonusPercent", fVictimAttackingPoiseBonusPercent}};
    SaveRaceFloatMap(poise, "BasePoise", basePoiseMap);
    SaveWeaponFloatMap(poise, "BasePoiseDamage", basePoiseDamageMap);
    SaveRaceFloatMap(poise, "BaseCreaturePoiseDamage", baseCreaturePoiseDamage);
    root["Poise"] = std::move(poise);

    root["Stagger"] = {{"UseStaggerSystem", bUseStaggerSystem},
                       {"StaggerRecoveryTimeSmall", uStaggerRecoveryTimeSmall},
                       {"StaggerRecoveryTimeMedium", uStaggerRecoveryTimeMedium},
                       {"StaggerRecoveryTimeLarge", uStaggerRecoveryTimeLarge}};

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

    json block = {{"UseBlockSystem", bUseBlockSystem},
                  {"TimedBlockEnabled", bTimedBlockEnabled},
                  {"TimedBlockNeverPostureBreak", bTimedBlockNeverPostureBreak},
                  {"TimedBlockLimit", uTimedBlockLimit},
                  {"TimedBlockDuration", uTimedBlockDuration},
                  {"BlockMaxStaminaConsumePercent", fBlockMaxStaminaConsumePercent},
                  {"BlockStaminaBonus", fBlockStaminaBonus},
                  {"TimedBlockBlockStrengthMult", fTimedBlockBlockStrengthMult}};
    SaveWeaponFloatMap(block, "BlockStrength", blockStrengthMap);
    root["Block"] = std::move(block);

    root["WeaponArt"] = {{"UseWeaponArtSystem", bUseWeaponArtSystem},
                         {"HideWeaponArtHUDOnSheathe", bHideWeaponArtHUDOnSheathe},
                         {"UseWeaponArtHUD", bUseWeaponArtHUD},
                         {"WeaponArtHUDPosX", fWeaponArtHUDPosX},
                         {"WeaponArtHUDPosY", fWeaponArtHUDPosY},
                         {"WeaponArtHUDScale", fWeaponArtHUDScale},
                         {"WeaponArtMenuStartPercent", fWeaponArtMenuStartPercent}};

    json execution = {{"UseExecutionSystem", bUseExecutionSystem},
                      {"OnHitDamageMultWhenExecutable", fOnHitDamageMultWhenExecutable}};
    SaveWeaponFloatMap(execution, "ExecutionDamageMult", executionDamageMultMap);
    root["Execution"] = std::move(execution);

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
  ClearSettingMaps();

  if (!EnsureSettingsDir())
    return;

  const auto settingsPath = std::string(SettingsFile);
  if (!std::filesystem::exists(settingsPath)) {
    logger::warn("Settings: settings file not found: {}", settingsPath);
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
    return;
  }

  if (!root.is_object()) {
    logger::warn("Settings: settings root is not a JSON object.");
    return;
  }

  if (const auto it = root.find("Damage"); it != root.end() && it->is_object()) {
    const auto& damage = *it;
    LoadSetting(damage, "Damage", "UseDamageSystem", bUseDamageSystem);
    LoadSetting(damage, "Damage", "DamageMultPowerAttack", fDamageMultPowerAttack);
    LoadSetting(damage, "Damage", "DamageMultBash", fDamageMultBash);
    LoadSetting(damage, "Damage", "DamageMultPowerBash", fDamageMultPowerBash);
  }

  if (const auto it = root.find("Stamina"); it != root.end() && it->is_object()) {
    const auto& stamina = *it;
    LoadSetting(stamina, "Stamina", "UseAttackStaminaSystem", bUseAttackStaminaSystem);
    LoadSetting(stamina, "Stamina", "ConsumeStaminaOutCombat", bConsumeStaminaOutCombat);
    LoadSetting(stamina, "Stamina", "DisableAttackWhenStaminaZero", bDisableAttackWhenStaminaZero);
    LoadSetting(stamina, "Stamina", "StaminaRegenMult", fStaminaRegenMult);
    LoadSetting(stamina, "Stamina", "StaminaRegenMin", fStaminaRegenMin);
    LoadSetting(stamina, "Stamina", "StaminaRegenMax", fStaminaRegenMax);
    LoadSetting(stamina, "Stamina", "StaminaRegenDelay", fStaminaRegenDelay);
    LoadSetting(stamina, "Stamina", "StaminaRegenMultCombat", fStaminaRegenMultCombat);
    LoadSetting(stamina, "Stamina", "StaminaRegenMultBlock", fStaminaRegenMultBlock);
    LoadSetting(stamina, "Stamina", "AttackStaminaCostPerMass", fAttackStaminaCostPerMass);
    LoadSetting(stamina, "Stamina", "PowerAttackStaminaCostMult", fPowerAttackStaminaCostMult);
    LoadSetting(stamina, "Stamina", "PowerAttackStaminaCostPerMass",
                fPowerAttackStaminaCostPerMass);
    LoadSetting(stamina, "Stamina", "BashStaminaCostMult", fBashStaminaCostMult);
    LoadSetting(stamina, "Stamina", "BashStaminaCostPerMass", fBashStaminaCostPerMass);
    LoadSetting(stamina, "Stamina", "PowerBashStaminaCostMult", fPowerBashStaminaCostMult);
    LoadSetting(stamina, "Stamina", "PowerBashStaminaCostPerMass", fPowerBashStaminaCostPerMass);
    LoadWeaponFloatMap(stamina, "Stamina", "BaseStaminaCost", baseStaminaCostMap);
    LoadRaceFloatMap(stamina, "Stamina", "BaseCreatureStamina", baseCreatureStaminaMap);
  }

  if (const auto it = root.find("Posture"); it != root.end() && it->is_object()) {
    const auto& posture = *it;
    LoadSetting(posture, "Posture", "UsePostureSystem", bUsePostureSystem);
    LoadSetting(posture, "Posture", "DisablePlayerPostureBreak", bDisablePlayerPostureBreak);
    LoadSetting(posture, "Posture", "UsePostureHUD", bUsePostureHUD);
    LoadSetting(posture, "Posture", "MaxPostureHealthMult", fMaxPostureHealthMult);
    LoadSetting(posture, "Posture", "MaxPostureStaminaMult", fMaxPostureStaminaMult);
    LoadSetting(posture, "Posture", "PostureRegenDelay", uPostureRegenDelay);
    LoadSetting(posture, "Posture", "PostureRegenPercentPerSecond", fPostureRegenPercentPerSecond);
    LoadSetting(posture, "Posture", "BashPostureDamageMult", fBashPostureDamageMult);
    LoadSetting(posture, "Posture", "PowerAttackPostureDamageMult", fPowerAttackPostureDamageMult);
    LoadSetting(posture, "Posture", "PowerBashPostureDamageMult", fPowerBashPostureDamageMult);
    LoadSetting(posture, "Posture", "ArmorPostureDamageFactor", fArmorPostureDamageFactor);
    LoadRaceFloatMap(posture, "Posture", "BasePosture", basePostureMap);
    LoadWeaponFloatMap(posture, "Posture", "BasePostureDamage", basePostureDamageMap);
    LoadRaceFloatMap(posture, "Posture", "BaseCreaturePostureDamage", baseCreaturePostureDamage);
  }

  if (const auto it = root.find("Poise"); it != root.end() && it->is_object()) {
    const auto& poise = *it;
    LoadSetting(poise, "Poise", "UsePoiseSystem", bUsePoiseSystem);
    LoadSetting(poise, "Poise", "PoiseStaminaMult", fPoiseStaminaMult);
    LoadSetting(poise, "Poise", "PoiseMassMult", fPoiseMassMult);
    LoadSetting(poise, "Poise", "PoiseRegenDelay", uPoiseRegenDelay);
    LoadSetting(poise, "Poise", "PoiseRegenPercentPerSecond", fPoiseRegenPercentPerSecond);
    LoadSetting(poise, "Poise", "StaggerCompensationPercent", fStaggerCompensationPercent);
    LoadSetting(poise, "Poise", "StaggerLevelSmall", fStaggerLevelSmall);
    LoadSetting(poise, "Poise", "StaggerLevelMedium", fStaggerLevelMedium);
    LoadSetting(poise, "Poise", "StaggerLevelLarge", fStaggerLevelLarge);
    LoadSetting(poise, "Poise", "LightArmorHeadMaxPoiseBonus", fLightArmorHeadMaxPoiseBonus);
    LoadSetting(poise, "Poise", "LightArmorBodyMaxPoiseBonus", fLightArmorBodyMaxPoiseBonus);
    LoadSetting(poise, "Poise", "LightArmorHandMaxPoiseBonus", fLightArmorHandMaxPoiseBonus);
    LoadSetting(poise, "Poise", "LightArmorFeetMaxPoiseBonus", fLightArmorFeetMaxPoiseBonus);
    LoadSetting(poise, "Poise", "HeavyArmorHeadMaxPoiseBonus", fHeavyArmorHeadMaxPoiseBonus);
    LoadSetting(poise, "Poise", "HeavyArmorBodyMaxPoiseBonus", fHeavyArmorBodyMaxPoiseBonus);
    LoadSetting(poise, "Poise", "HeavyArmorHandMaxPoiseBonus", fHeavyArmorHandMaxPoiseBonus);
    LoadSetting(poise, "Poise", "HeavyArmorFeetMaxPoiseBonus", fHeavyArmorFeetMaxPoiseBonus);
    LoadSetting(poise, "Poise", "BashPoiseDamageMult", fBashPoiseDamageMult);
    LoadSetting(poise, "Poise", "LevelDiffAggressorMultPerLevel", fLevelDiffAggressorMultPerLevel);
    LoadSetting(poise, "Poise", "VictimAttackingPoiseBonusPercent",
                fVictimAttackingPoiseBonusPercent);
    LoadSetting(poise, "Poise", "PowerAttackPoiseDamageMult", fPowerAttackPoiseDamageMult);
    LoadSetting(poise, "Poise", "PowerBashPoiseDamageMult", fPowerBashPoiseDamageMult);
    LoadRaceFloatMap(poise, "Poise", "BasePoise", basePoiseMap);
    LoadWeaponFloatMap(poise, "Poise", "BasePoiseDamage", basePoiseDamageMap);
    LoadRaceFloatMap(poise, "Poise", "BaseCreaturePoiseDamage", baseCreaturePoiseDamage);
  }

  if (const auto it = root.find("Stagger"); it != root.end() && it->is_object()) {
    const auto& stagger = *it;
    LoadSetting(stagger, "Stagger", "UseStaggerSystem", bUseStaggerSystem);
    LoadSetting(stagger, "Stagger", "StaggerRecoveryTimeSmall", uStaggerRecoveryTimeSmall);
    LoadSetting(stagger, "Stagger", "StaggerRecoveryTimeMedium", uStaggerRecoveryTimeMedium);
    LoadSetting(stagger, "Stagger", "StaggerRecoveryTimeLarge", uStaggerRecoveryTimeLarge);
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
    LoadSetting(block, "Block", "TimedBlockDuration", uTimedBlockDuration);
    LoadSetting(block, "Block", "BlockMaxStaminaConsumePercent", fBlockMaxStaminaConsumePercent);
    LoadSetting(block, "Block", "BlockStaminaBonus", fBlockStaminaBonus);
    LoadSetting(block, "Block", "TimedBlockBlockStrengthMult", fTimedBlockBlockStrengthMult);
    LoadWeaponFloatMap(block, "Block", "BlockStrength", blockStrengthMap);
  }

  if (const auto it = root.find("WeaponArt"); it != root.end() && it->is_object()) {
    const auto& weaponArt = *it;
    LoadSetting(weaponArt, "WeaponArt", "HideWeaponArtHUDOnSheathe", bHideWeaponArtHUDOnSheathe);
    LoadSetting(weaponArt, "WeaponArt", "UseWeaponArtSystem", bUseWeaponArtSystem);
    LoadSetting(weaponArt, "WeaponArt", "UseWeaponArtHUD", bUseWeaponArtHUD);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtHUDPosX", fWeaponArtHUDPosX);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtHUDPosY", fWeaponArtHUDPosY);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtHUDScale", fWeaponArtHUDScale);
    LoadSetting(weaponArt, "WeaponArt", "WeaponArtMenuStartPercent", fWeaponArtMenuStartPercent);
  }

  if (const auto it = root.find("Execution"); it != root.end() && it->is_object()) {
    const auto& execution = *it;
    LoadSetting(execution, "Execution", "UseExecutionSystem", bUseExecutionSystem);
    LoadSetting(execution, "Execution", "OnHitDamageMultWhenExecutable",
                fOnHitDamageMultWhenExecutable);
    LoadWeaponFloatMap(execution, "Execution", "ExecutionDamageMult", executionDamageMultMap);
  }
}

void SaveSettings()
{
  UpdateGameSettings();
  WriteSettingsFile();
}

void ResetSettings()
{
  if (!EnsureSettingsDir())
    return;

  if (!OverwriteSettingsFromDefault())
    return;

  LoadSettings();
  UpdateGameSettings();
}
}  // namespace Settings