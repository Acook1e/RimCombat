#include "Settings.h"

#include "Utils.h"

namespace Settings
{

void SetStaminaRegen(float mult, float upperLimit, float lowerLimit, bool restore = false)
{
  static std::unordered_map<RE::TESRace*, float> originalStaminaRegen;
  if (restore) {
    if (originalStaminaRegen.empty())
      return;
    for (auto& [race, originalRegen] : originalStaminaRegen)
      race->data.staminaRegen = originalRegen;
    // originalStaminaRegen.clear();
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
  SettingsDir = "Data/SKSE/Plugins/" + std::string(SKSE::GetPluginName()) + "/";

  std::error_code ec;
  if (!std::filesystem::exists(SettingsDir, ec)) {
    logger::info("Settings directory does not exist, skipping loading settings.");
    return;
  }

  constexpr std::string_view settingsFileName = "settings.ini";
}

void SaveSettings()
{
  UpdateGameSettings();
}
}  // namespace Settings