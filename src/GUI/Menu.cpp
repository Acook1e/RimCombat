#include "GUI/Menu.h"

#include "GUI/Localization.h"
#include "GUI/UI.h"
#include "Utils.h"

#include "API/SKSEMenuFramework.h"

namespace ImGui
{
void ShowTooltip(const Localization::Entry& map)
{
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}

void SetSection(std::uint32_t hash)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (!map.label.empty())
    SKSEMenuFramework::SetSection(map.label.data());
  else
    SKSEMenuFramework::SetSection(Localization::GetLocalization("Unknown"_h).label.data());
}

void AddSectionItem(std::uint32_t hash, SKSEMenuFramework::Model::RenderFunction func)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (!map.label.empty())
    SKSEMenuFramework::AddSectionItem(map.label.data(), func);
}

void Text(std::uint32_t hash, auto&&... args)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (!map.label.empty()) {
    ImGuiMCP::Text(std::vformat(map.label, std::make_format_args(args...)).data());
    ShowTooltip(map);
  }
}

void Checkbox(std::uint32_t hash, bool* v, std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (ImGuiMCP::Checkbox(map.label.data(), v))
    if (onChange)
      onChange();
  ShowTooltip(map);
}

void Button(std::uint32_t hash, std::function<void()> onClick = nullptr)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (ImGuiMCP::Button(map.label.data()))
    if (onClick)
      onClick();
  ShowTooltip(map);
}

void DragInt(std::uint32_t hash, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0,
             const char* format = "%d", std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map   = Localization::GetLocalization(hash);
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  if (ImGuiMCP::DragInt(map.label.data(), v, v_speed, v_min, v_max, format, flags))
    if (onChange)
      onChange();
  ShowTooltip(map);
}

void DragUInt64(std::uint32_t hash, std::uint64_t* v, float v_speed = 1.0f, int v_min = 0,
                int v_max = 0, const char* format = "%d", std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map   = Localization::GetLocalization(hash);
  auto value                       = static_cast<int>(*v);
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  if (ImGuiMCP::DragInt(map.label.data(), &value, v_speed, v_min, v_max, format, flags)) {
    if (value < v_min)
      value = v_min;
    *v = static_cast<std::uint64_t>(value);
    if (onChange)
      onChange();
  }
  ShowTooltip(map);
}

void DragFloat(std::uint32_t hash, float* v, float v_speed = 1.0f, float v_min = 0.0f,
               float v_max = 0.0f, const char* format = "%.1f",
               std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map   = Localization::GetLocalization(hash);
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  if (ImGuiMCP::DragFloat(map.label.data(), v, v_speed, v_min, v_max, format, flags))
    if (onChange)
      onChange();
  ShowTooltip(map);
}

template <typename T>
void Combo(std::uint32_t hash, std::vector<std::reference_wrapper<Localization::Entry>> comboIt,
           T* current_item, std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (ImGuiMCP::BeginCombo(map.label.data(), comboIt[*current_item].label.data())) {
    for (size_t n = 0; n < comboIt.size(); n++) {
      const bool is_selected = (*current_item == static_cast<int>(n));
      if (ImGuiMCP::Selectable(comboIt[n].get().label.data(), is_selected)) {
        *current_item = static_cast<int>(n);
        if (onChange)
          onChange();
      }
      if (ImGuiMCP::IsItemHovered() && !comboIt[n].get().desc.empty())
        ImGuiMCP::SetTooltip(comboIt[n].get().desc.data());
      if (is_selected)
        ImGuiMCP::SetItemDefaultFocus();
    }
    ImGuiMCP::EndCombo();
  }
}
}  // namespace ImGui

namespace MenuImpl
{

void General()
{
  ImGui::Button("LoadSettings"_h, []() {
    Settings::LoadSettings();
    Settings::UpdateGameSettings();
  });

  ImGui::Button("SaveSettings"_h, []() {
    Settings::SaveSettings();
  });

  ImGui::Button("ResetSettings"_h, []() {
    Settings::ResetSettings();
    Settings::UpdateGameSettings();
  });

  ImGui::Text("MenuGeneralSummary"_h);
}

void Damage()
{
  ImGui::Text("MenuDamageSummary"_h);
  ImGui::Checkbox("UseDamageSystem"_h, &Settings::bUseDamageSystem);
  ImGui::DragFloat("DamageMultPowerAttack"_h, &Settings::fDamageMultPowerAttack, 0.05f, 0.0f, 10.0f,
                   "%.2f");
  ImGui::DragFloat("DamageMultBash"_h, &Settings::fDamageMultBash, 0.05f, 0.0f, 10.0f, "%.2f");
  ImGui::DragFloat("DamageMultPowerBash"_h, &Settings::fDamageMultPowerBash, 0.05f, 0.0f, 10.0f,
                   "%.2f");
}

void Stamina()
{
  ImGui::Text("MenuStaminaSummary"_h);
  ImGui::Checkbox("UseAttackStaminaSystem"_h, &Settings::bUseAttackStaminaSystem);
  ImGui::Checkbox("ConsumeStaminaOutCombat"_h, &Settings::bConsumeStaminaOutCombat);
  ImGui::Checkbox("DisableAttackWhenStaminaZero"_h, &Settings::bDisableAttackWhenStaminaZero);
  ImGui::DragFloat("StaminaRegenMult"_h, &Settings::fStaminaRegenMult, 0.05f, 0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("StaminaRegenMin"_h, &Settings::fStaminaRegenMin, 0.5f, 0.0f, 200.0f, "%.1f");
  ImGui::DragFloat("StaminaRegenMax"_h, &Settings::fStaminaRegenMax, 0.5f, 0.0f, 200.0f, "%.1f");
  ImGui::DragFloat("StaminaRegenDelay"_h, &Settings::fStaminaRegenDelay, 0.05f, 0.0f, 10.0f,
                   "%.2f");
  ImGui::DragFloat("StaminaRegenMultCombat"_h, &Settings::fStaminaRegenMultCombat, 0.05f, 0.0f,
                   20.0f, "%.2f");
  ImGui::DragFloat("StaminaRegenMultBlock"_h, &Settings::fStaminaRegenMultBlock, 0.05f, 0.0f, 20.0f,
                   "%.2f");
  ImGui::DragFloat("AttackStaminaCostPerMass"_h, &Settings::fAttackStaminaCostPerMass, 0.01f, 0.0f,
                   10.0f, "%.2f");
  ImGui::DragFloat("PowerAttackStaminaCostMult"_h, &Settings::fPowerAttackStaminaCostMult, 0.05f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("PowerAttackStaminaCostPerMass"_h, &Settings::fPowerAttackStaminaCostPerMass,
                   0.01f, 0.0f, 10.0f, "%.2f");
  ImGui::DragFloat("BashStaminaCostMult"_h, &Settings::fBashStaminaCostMult, 0.05f, 0.0f, 10.0f,
                   "%.2f");
  ImGui::DragFloat("BashStaminaCostPerMass"_h, &Settings::fBashStaminaCostPerMass, 0.01f, 0.0f,
                   10.0f, "%.2f");
  ImGui::DragFloat("PowerBashStaminaCostMult"_h, &Settings::fPowerBashStaminaCostMult, 0.05f, 0.0f,
                   10.0f, "%.2f");
  ImGui::DragFloat("PowerBashStaminaCostPerMass"_h, &Settings::fPowerBashStaminaCostPerMass, 0.01f,
                   0.0f, 10.0f, "%.2f");
}

void Posture()
{
  ImGui::Text("MenuPostureSummary"_h);
  ImGui::Checkbox("UsePostureSystem"_h, &Settings::bUsePostureSystem);
  ImGui::Checkbox("UsePostureHUD"_h, &Settings::bUsePostureHUD);
  ImGui::Checkbox("DisablePlayerPostureBreak"_h, &Settings::bDisablePlayerPostureBreak);
  ImGui::DragFloat("MaxPostureHealthMult"_h, &Settings::fMaxPostureHealthMult, 0.005f, 0.0f, 1.0f,
                   "%.3f");
  ImGui::DragFloat("MaxPostureStaminaMult"_h, &Settings::fMaxPostureStaminaMult, 0.005f, 0.0f, 1.0f,
                   "%.3f");
  ImGui::DragUInt64("PostureRegenDelay"_h, &Settings::uPostureRegenDelay, 10.0f, 0, 60000);
  ImGui::DragFloat("PostureRegenPercentPerSecond"_h, &Settings::fPostureRegenPercentPerSecond, 0.1f,
                   0.0f, 100.0f, "%.1f");
  ImGui::DragFloat("BashPostureDamageMult"_h, &Settings::fBashPostureDamageMult, 0.05f, 0.0f, 20.0f,
                   "%.2f");
  ImGui::DragFloat("PowerAttackPostureDamageMult"_h, &Settings::fPowerAttackPostureDamageMult,
                   0.05f, 0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("PowerBashPostureDamageMult"_h, &Settings::fPowerBashPostureDamageMult, 0.05f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("ArmorPostureDamageFactor"_h, &Settings::fArmorPostureDamageFactor, 0.01f, 0.0f,
                   10.0f, "%.2f");
}

void Poise()
{
  ImGui::Text("MenuPoiseSummary"_h);
  ImGui::Checkbox("UsePoiseSystem"_h, &Settings::bUsePoiseSystem);
  ImGui::DragFloat("PoiseStaminaMult"_h, &Settings::fPoiseStaminaMult, 0.0001f, 0.0f, 1.0f, "%.4f");
  ImGui::DragFloat("PoiseMassMult"_h, &Settings::fPoiseMassMult, 0.01f, 0.0f, 20.0f, "%.2f");
  ImGui::DragUInt64("PoiseRegenDelay"_h, &Settings::uPoiseRegenDelay, 10.0f, 0, 60000);
  ImGui::DragFloat("PoiseRegenPercentPerSecond"_h, &Settings::fPoiseRegenPercentPerSecond, 0.1f,
                   0.0f, 100.0f, "%.1f");
  ImGui::DragFloat("StaggerCompensationPercent"_h, &Settings::fStaggerCompensationPercent, 0.01f,
                   0.0f, 1.0f, "%.2f");
  ImGui::DragFloat("StaggerLevelSmall"_h, &Settings::fStaggerLevelSmall, 0.05f, 0.0f, 100.0f,
                   "%.2f");
  ImGui::DragFloat("StaggerLevelMedium"_h, &Settings::fStaggerLevelMedium, 0.05f, 0.0f, 100.0f,
                   "%.2f");
  ImGui::DragFloat("StaggerLevelLarge"_h, &Settings::fStaggerLevelLarge, 0.05f, 0.0f, 100.0f,
                   "%.2f");
  ImGui::DragFloat("LightArmorHeadMaxPoiseBonus"_h, &Settings::fLightArmorHeadMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("LightArmorBodyMaxPoiseBonus"_h, &Settings::fLightArmorBodyMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("LightArmorHandMaxPoiseBonus"_h, &Settings::fLightArmorHandMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("LightArmorFeetMaxPoiseBonus"_h, &Settings::fLightArmorFeetMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("HeavyArmorHeadMaxPoiseBonus"_h, &Settings::fHeavyArmorHeadMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("HeavyArmorBodyMaxPoiseBonus"_h, &Settings::fHeavyArmorBodyMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("HeavyArmorHandMaxPoiseBonus"_h, &Settings::fHeavyArmorHandMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("HeavyArmorFeetMaxPoiseBonus"_h, &Settings::fHeavyArmorFeetMaxPoiseBonus, 0.01f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("BashPoiseDamageMult"_h, &Settings::fBashPoiseDamageMult, 0.05f, 0.0f, 20.0f,
                   "%.2f");
  ImGui::DragFloat("PowerAttackPoiseDamageMult"_h, &Settings::fPowerAttackPoiseDamageMult, 0.05f,
                   0.0f, 20.0f, "%.2f");
  ImGui::DragFloat("PowerBashPoiseDamageMult"_h, &Settings::fPowerBashPoiseDamageMult, 0.05f, 0.0f,
                   20.0f, "%.2f");
  ImGui::DragFloat("LevelDiffAggressorMultPerLevel"_h, &Settings::fLevelDiffAggressorMultPerLevel,
                   0.005f, 0.0f, 1.0f, "%.3f");
  ImGui::DragFloat("VictimAttackingPoiseBonusPercent"_h,
                   &Settings::fVictimAttackingPoiseBonusPercent, 0.05f, 0.0f, 2.0f, "%.2f");
}

void Stagger()
{
  ImGui::Checkbox("UseStaggerSystem"_h, &Settings::bUseStaggerSystem);
  ImGui::DragUInt64("StaggerRecoveryTimeSmall"_h, &Settings::uStaggerRecoveryTimeSmall, 10.0f, 0,
                    10000);
  ImGui::DragUInt64("StaggerRecoveryTimeMedium"_h, &Settings::uStaggerRecoveryTimeMedium, 10.0f, 0,
                    10000);
  ImGui::DragUInt64("StaggerRecoveryTimeLarge"_h, &Settings::uStaggerRecoveryTimeLarge, 10.0f, 0,
                    10000);
}

void Exhausted()
{
  ImGui::Checkbox("UseExhaustedSystem"_h, &Settings::bUseExhaustedSystem);
  ImGui::Checkbox("DisablePlayerAttackWhenExhausted"_h,
                  &Settings::bDisablePlayerAttackWhenExhausted);
  ImGui::Checkbox("DisableNPCAttackWhenExhausted"_h, &Settings::bDisableNPCAttackWhenExhausted);
  ImGui::Checkbox("ExitExhaustedOnHit"_h, &Settings::bExitExhaustedOnHit);
  ImGui::DragFloat("ExhaustedExitPercent"_h, &Settings::fExhaustedExitPercent, 0.01f, 0.0f, 1.0f,
                   "%.2f");
  ImGui::DragFloat("AttackDamageMultWhenExhausted"_h, &Settings::fAttackDamageMultWhenExhausted,
                   0.05f, 0.0f, 10.0f, "%.2f");
  ImGui::DragFloat("AttackPostureDamageMultWhenExhausted"_h,
                   &Settings::fAttackPostureDamageMultWhenExhausted, 0.05f, 0.0f, 10.0f, "%.2f");
  ImGui::DragFloat("OnHitDamageMultWhenExhausted"_h, &Settings::fOnHitDamageMultWhenExhausted,
                   0.05f, 0.0f, 10.0f, "%.2f");
  ImGui::DragFloat("OnHitPostureDamageMultWhenExhausted"_h,
                   &Settings::fOnHitPostureDamageMultWhenExhausted, 0.05f, 0.0f, 10.0f, "%.2f");
}

void Block()
{
  ImGui::Text("MenuBlockSummary"_h);
  ImGui::Checkbox("UseBlockSystem"_h, &Settings::bUseBlockSystem);
  ImGui::Checkbox("TimedBlockEnabled"_h, &Settings::bTimedBlockEnabled);
  ImGui::Checkbox("TimedBlockNeverPostureBreak"_h, &Settings::bTimedBlockNeverPostureBreak);
  ImGui::DragFloat("TimedBlockPostureDamageReflectMult"_h,
                   &Settings::fTimedBlockPostureDamageReflectMult, 0.01f, 0.0f, 2.0f, "%.2f");
  ImGui::Checkbox("TimedBlockReflectPostureBreak"_h,
                  &Settings::bTimedBlockReflectPostureBreak);
  ImGui::DragUInt64("TimedBlockLimit"_h, &Settings::uTimedBlockLimit, 5.0f, 0, 5000);
  ImGui::DragUInt64("TimedBlockDuration"_h, &Settings::uTimedBlockDuration, 5.0f, 0, 10000);
  ImGui::DragFloat("BlockMaxStaminaConsumePercent"_h, &Settings::fBlockMaxStaminaConsumePercent,
                   0.01f, 0.0f, 1.0f, "%.2f");
  ImGui::DragFloat("BlockStaminaBonus"_h, &Settings::fBlockStaminaBonus, 0.1f, 0.0f, 1000.0f,
                   "%.1f");
  ImGui::DragFloat("TimedBlockBlockStrengthMult"_h, &Settings::fTimedBlockBlockStrengthMult, 0.05f,
                   0.0f, 20.0f, "%.2f");
}

void WeaponArt()
{
  ImGui::Checkbox("UseWeaponArtSystem"_h, &Settings::bUseWeaponArtSystem, []() {
    if (!Settings::bUseWeaponArtSystem) {
      Settings::bUseWeaponArtHUD = false;
      UI::WeaponArtHUD::Hide();
    }
  });
  ImGui::Checkbox("HideWeaponArtHUDOnSheathe"_h, &Settings::bHideWeaponArtHUDOnSheathe);
  ImGui::Checkbox("UseWeaponArtHUD"_h, &Settings::bUseWeaponArtHUD, []() {
    if (!Settings::bUseWeaponArtSystem)
      Settings::bUseWeaponArtHUD = false;
    if (!Settings::bUseWeaponArtHUD)
      UI::WeaponArtHUD::Hide();
  });
  ImGui::DragFloat("WeaponArtHUDPosX"_h, &Settings::fWeaponArtHUDPosX, 0.1f, 0.0f, 100.0f, "%.1f");
  ImGui::DragFloat("WeaponArtHUDPosY"_h, &Settings::fWeaponArtHUDPosY, 0.1f, 0.0f, 100.0f, "%.1f");
  ImGui::DragFloat("WeaponArtHUDScale"_h, &Settings::fWeaponArtHUDScale, 0.01f, 0.1f, 10.0f,
                   "%.2f");
  ImGui::DragFloat("WeaponArtMenuStartPercent"_h, &Settings::fWeaponArtMenuStartPercent, 0.1f, 0.0f,
                   100.0f, "%.1f");
}

void Execution()
{
  ImGui::Text("MenuExecutionSummary"_h);
  ImGui::Checkbox("UseExecutionSystem"_h, &Settings::bUseExecutionSystem);
  ImGui::DragFloat("OnHitDamageMultWhenExecutable"_h, &Settings::fOnHitDamageMultWhenExecutable,
                   0.05f, 0.0f, 10.0f, "%.2f");
}

void Debug()
{
  auto cross        = RE::CrosshairPickData::GetSingleton();
  RE::Actor* target = nullptr;
  try {
    if (auto targetActor = cross->targetActor.get(); targetActor.get())
      target = targetActor.get()->As<RE::Actor>();
    else
      target = RE::PlayerCharacter::GetSingleton();
  } catch (const std::exception& e) {
    logger::warn("{}", e.what());
  }

  auto none = Localization::GetLocalization("None"_h).label.data();
  ImGui::Text("DebugCrosshairTarget"_h, target ? target->GetDisplayFullName() : none);
}

SKSEMenuFramework::Model::Event* event = nullptr;
}  // namespace MenuImpl

Menu::Menu()
{
  if (!SKSEMenuFramework::IsInstalled())
    return;

  ImGui::SetSection("RimCombat"_h);

  ImGui::AddSectionItem("General"_h, MenuImpl::General);
  ImGui::AddSectionItem("Stamina"_h, MenuImpl::Stamina);
  ImGui::AddSectionItem("Damage"_h, MenuImpl::Damage);
  ImGui::AddSectionItem("Posture"_h, MenuImpl::Posture);
  ImGui::AddSectionItem("Poise"_h, MenuImpl::Poise);
  ImGui::AddSectionItem("Stagger"_h, MenuImpl::Stagger);
  ImGui::AddSectionItem("Exhausted"_h, MenuImpl::Exhausted);
  ImGui::AddSectionItem("Block"_h, MenuImpl::Block);
  ImGui::AddSectionItem("WeaponArt"_h, MenuImpl::WeaponArt);
  ImGui::AddSectionItem("Execution"_h, MenuImpl::Execution);
  ImGui::AddSectionItem("Debug"_h, MenuImpl::Debug);

  auto callback = [](SKSEMenuFramework::Model::EventType eventType) {
    switch (eventType) {
    case SKSEMenuFramework::Model::EventType::kOpenMenu:
      break;
    case SKSEMenuFramework::Model::EventType::kCloseMenu:
      // 关闭菜单自动保存设置
      Settings::SaveSettings();
      break;
    }
  };
  MenuImpl::event = new SKSEMenuFramework::Model::Event(callback, static_cast<float>(MOD));

  logger::info("Menu: SKSEMenuFramework v{} loaded.", SKSEMenuFramework::GetMenuFrameworkVersion());
}

Menu::~Menu()
{
  if (MenuImpl::event)
    delete MenuImpl::event;
  MenuImpl::event = nullptr;
}
