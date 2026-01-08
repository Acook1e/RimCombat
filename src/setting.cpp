#include "setting.h"

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

void SetStaminaRegen(float mult, float upperLimit, float lowerLimit)
{
  for (auto& race : RE::TESDataHandler::GetSingleton()->GetFormArray<RE::TESRace>()) {
    if (race) {
      float staminaRegen      = race->data.staminaRegen * mult;
      staminaRegen            = min(staminaRegen, upperLimit);
      staminaRegen            = max(staminaRegen, lowerLimit);
      race->data.staminaRegen = staminaRegen;
    }
  }
}

void InitSettings()
{
  SetStaminaRegen(Settings::fStaminaRegenMult, Settings::fStaminaRegenLimit, Settings::fStaminaRegenMin);
  SetGameSettings("fDamagedStaminaRegenDelay", fStaminaRegenDelay);
  SetGameSettings("fCombatStaminaRegenRateMult", fCombatStaminaRegenMult);
}

void LoadSettings() {}

void SaveSettings() {}
}  // namespace Settings