#include "Menu.h"
#include "Posture.h"
#include "Utils.h"

#include "simpleini.h"

using Utils::operator""_h;

namespace Localization
{
struct StringMap
{
  std::string label;
  std::string desc;
};

static std::unordered_map<std::uint32_t, StringMap> stringMaps;

void LoadLocalization()
{
  constexpr std::string_view iniName = "Localization.ini";
  constexpr std::string_view section = "Localization";

  CSimpleIniA ini;
  ini.SetUnicode();
  SI_Error rc = ini.LoadFile((Settings::SettingsDir + std::string(iniName)).data());

  auto InsertValue = [&](const char* key) {
    std::string keyStr(key);
    if (keyStr.ends_with("_DESC"))
      return;
    auto label                                            = ini.GetValue(section.data(), key, "");
    auto desc                                             = ini.GetValue(section.data(), (keyStr + "_DESC").data(), "");
    stringMaps[Utils::hash(keyStr.data(), keyStr.size())] = {label, desc};
  };

  if (rc < 0) {
    logger::error("Failed to load localization file: {}. Error code: {}", iniName, rc);
    return;
  }

  CSimpleIniA::TNamesDepend keys;
  ini.GetAllKeys(section.data(), keys);
  for (const auto& key : keys)
    InsertValue(key.pItem);
}
}  // namespace Localization

namespace ImGui
{
using StringMap = Localization::StringMap;
inline StringMap GetStringMap(std::uint32_t hash)
{
  auto it = Localization::stringMaps.find(hash);
  if (it != Localization::stringMaps.end())
    return it->second;
  else
    return {"", ""};
}
void SetSection(std::uint32_t hash)
{
  StringMap map = GetStringMap(hash);
  if (!map.label.empty())
    SKSEMenuFramework::SetSection(map.label.data());
  else
    SKSEMenuFramework::SetSection(GetStringMap("UnknownValue"_h).label.data());
}
void AddSectionItem(std::uint32_t hash, SKSEMenuFramework::Model::RenderFunction func)
{
  StringMap map = GetStringMap(hash);
  if (!map.label.empty())
    SKSEMenuFramework::AddSectionItem(map.label.data(), func);
  else
    SKSEMenuFramework::AddSectionItem(GetStringMap("UnknownValue"_h).label.data(), func);
}
void Checkbox(std::uint32_t hash, bool* v, std::function<void()> onChange = nullptr)
{
  StringMap map = GetStringMap(hash);
  if (ImGuiMCP::Checkbox(map.label.data(), v))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
void DragFloat(std::uint32_t hash, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.1f",
               std::function<void()> onChange = nullptr)
{
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  StringMap map                    = GetStringMap(hash);
  if (ImGuiMCP::DragFloat(map.label.data(), v, v_speed, v_min, v_max, format, flags))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
}  // namespace ImGui

void Menu::Stamina()
{
  ImGui::Checkbox("bUseAttackStaminaSystem"_h, &Settings::bUseAttackStaminaSystem);
  ImGui::Checkbox("bConsumeStaminaOutCombat"_h, &Settings::bConsumeStaminaOutCombat);

  ImGuiMCP::Separator();

  ImGui::DragFloat("fStaminaRegenMult"_h, &Settings::fStaminaRegenMult, 0.1f, 0.1f, 10.0f);
  ImGui::DragFloat("fStaminaRegenMultCombat"_h, &Settings::fStaminaRegenMultCombat, 0.1f, 0.1f, 1.0f);
  ImGui::DragFloat("fStaminaRegenMultBlock"_h, &Settings::fStaminaRegenMultBlock, 0.1f, 0.1f, 1.0f);
  ImGui::DragFloat("fStaminaRegenMin"_h, &Settings::fStaminaRegenMin, 1.0f, 10.0f, Settings::fStaminaRegenMax);
  ImGui::DragFloat("fStaminaRegenMax"_h, &Settings::fStaminaRegenMax, 1.0f, Settings::fStaminaRegenMin, 100.0f);
  ImGui::DragFloat("fStaminaRegenDelay"_h, &Settings::fStaminaRegenDelay, 1.0f, 0.1f, 5.0f);

  ImGuiMCP::Separator();

  ImGui::Checkbox("bNormalAttackComsumeStamina"_h, &Settings::bNormalAttackComsumeStamina);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_Fist"_h, &Settings::fNormalAttackStaminaCostBase_Fist, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_Dagger"_h, &Settings::fNormalAttackStaminaCostBase_Dagger, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_Sword"_h, &Settings::fNormalAttackStaminaCostBase_Sword, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_Axe"_h, &Settings::fNormalAttackStaminaCostBase_Axe, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_Mace"_h, &Settings::fNormalAttackStaminaCostBase_Mace, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_GreatSword"_h, &Settings::fNormalAttackStaminaCostBase_GreatSword, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostBase_GreatAxe"_h, &Settings::fNormalAttackStaminaCostBase_GreatAxe, 0.1f, 0.0f, 100.0f);
  ImGui::DragFloat("fPowerAttackStaminaCostMult"_h, &Settings::fPowerAttackStaminaCostMult, 0.1f, 0.0f, 10.0f);
  ImGui::DragFloat("fNormalAttackStaminaCostPerMass"_h, &Settings::fNormalAttackStaminaCostPerMass, 0.01f, 0.0f, 10.0f);
  ImGui::DragFloat("fPowerAttackStaminaCostPerMass"_h, &Settings::fPowerAttackStaminaCostPerMass, 0.01f, 0.0f, 10.0f);

  ImGuiMCP::Separator();
}

void Menu::Posture()
{
  ImGui::Checkbox("bUsePostureSystem"_h, &Settings::bUsePostureSystem, []() {
    if (!Settings::bUsePostureSystem) {
      Settings::bUsePostureHUD = false;
      Posture::GetSingleton().ReleaseHUD();
    } else {
      Settings::bUsePostureHUD = true;
      Posture::GetSingleton().InitHUD();
    }
  });
  ImGui::Checkbox("bUsePostureHUD"_h, &Settings::bUsePostureHUD, []() {
    if (!Settings::bUsePostureHUD)
      Posture::GetSingleton().ReleaseHUD();
    else
      Posture::GetSingleton().InitHUD();
  });

  ImGuiMCP::Separator();

  ImGui::DragFloat("fMaxPostureBase"_h, &Settings::fMaxPostureBase, 1.0f, 50.0f, 200.0f);
  ImGui::DragFloat("fMaxPostureHealthMult"_h, &Settings::fMaxPostureHealthMult, 0.01f, 0.0f, 1.0f);
  ImGui::DragFloat("fArmorPostureDamageFactor"_h, &Settings::fArmorPostureDamageFactor, 0.1f, 0.1f, 10.0f);

  ImGuiMCP::Separator();

  ImGui::Checkbox("bEnableExhausted"_h, &Settings::bEnableExhausted);
  ImGui::DragFloat("fExhaustedRestorePercent"_h, &Settings::fExhaustedRestorePercent, 0.01f, 0.0f, 1.0f, "%.2f%%");
  ImGui::DragFloat("fExhaustedPostureDamageMult"_h, &Settings::fExhaustedPostureDamageMult, 0.1f, 1.0f, 5.0f);
}

void __stdcall Menu::EventListener(SKSEMenuFramework::Model::EventType eventType)
{
  switch (eventType) {
  case SKSEMenuFramework::Model::EventType::kOpenMenu:
    break;
  case SKSEMenuFramework::Model::EventType::kCloseMenu:
    Settings::SaveSettings();
    break;
  }
}

Menu::Menu()
{
  Localization::LoadLocalization();

  if (!SKSEMenuFramework::IsInstalled())
    return;

  ImGui::SetSection("RimCombat"_h);

  ImGui::AddSectionItem("Stamina"_h, Stamina);
  ImGui::AddSectionItem("Posture"_h, Posture);

  ImGui::AddSectionItem("No Exist Value"_h, Posture);

  // priority should be a individual value for each mod, here is nexus id of this mod
  event = new SKSEMenuFramework::Model::Event(EventListener, nexusID);

  logger::info("Menu: SKSEMenuFramework v{} loaded.", SKSEMenuFramework::GetMenuFrameworkVersion());
}