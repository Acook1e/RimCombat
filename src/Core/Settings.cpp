#include "Core/Settings.h"

#include "Utils.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using json = nlohmann::json;

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
  std::error_code ec;
  if (!std::filesystem::exists(SettingsDir, ec)) {
    std::filesystem::create_directories(SettingsDir, ec);
  }

  if (ec) {
    logger::warn("Settings: failed to create settings directory.");
    return;
  }
}

void SaveSettings()
{
  UpdateGameSettings();

  std::error_code ec;
  if (!std::filesystem::exists(SettingsDir, ec)) {
    std::filesystem::create_directories(SettingsDir, ec);
  }

  if (ec) {
    logger::warn("Settings: failed to create settings directory.");
    return;
  }
}
}  // namespace Settings