#include "Settings.h"

#include "Utils.h"

namespace Settings
{
static std::unordered_map<std::uint32_t, std::int32_t> hashMap;

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
  // Global settings
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

void AddHashMapping(uint32_t hash, uint32_t prefix)
{
  hashMap[hash] = prefix;
}

uint64_t toPersistForm(uint32_t hash, uint32_t suffix)
{
  return (static_cast<uint64_t>(hash) << 32) | suffix;
}

RE::TESForm* toTESForm(uint64_t form)
{
  uint32_t hash   = form >> 32;
  uint32_t suffix = form & 0x00000000FFFFFFFF;
  auto it         = hashMap.find(hash);
  if (it == hashMap.end()) {
    logger::warn("Hash {} not found in hashMap", hash);
    return nullptr;
  }
  return reinterpret_cast<RE::TESForm*>(it->second | suffix);
}
}  // namespace Settings