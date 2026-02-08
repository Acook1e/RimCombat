#include "Settings.h"

namespace Settings
{
template <typename T>
void SetGameSettings(const char* a_setting, T a_value)
{
  RE::Setting* setting                          = nullptr;
  RE::GameSettingCollection* _settingCollection = RE::GameSettingCollection::GetSingleton();
  setting                                       = _settingCollection->GetSetting(a_setting);
  if (!setting) {
    logger::info("SetGameSetting: Invalid setting: {}", a_setting);
  } else {
    if constexpr (std::is_same_v<T, bool>) {
      setting->data.b = a_value;
    } else if constexpr (std::is_same_v<T, float>) {
      setting->data.f = a_value;
    } else if constexpr (std::is_same_v<T, std::int32_t>) {
      setting->data.i = a_value;
    } else if constexpr (std::is_same_v<T, RE::Color>) {
      setting->data.r = a_value;
    } else if constexpr (std::is_same_v<T, char*>) {
      setting->data.s = a_value;
    } else if constexpr (std::is_same_v<T, std::uint32_t>) {
      setting->data.u = a_value;
    }
  }
}

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

void RefreshGameSettings()
{
  SetStaminaRegen(fStaminaRegenMult, fStaminaRegenMax, fStaminaRegenMin, fStaminaRegenMult == 1.0f);
  SetGameSettings("fDamagedStaminaRegenDelay", fStaminaRegenDelay);
  SetGameSettings("fCombatStaminaRegenRateMult", fStaminaRegenMultCombat);
}

void LoadSettings()
{
  RefreshGameSettings();
}

void SaveSettings()
{
  RefreshGameSettings();
}
}  // namespace Settings