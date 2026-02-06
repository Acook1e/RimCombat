#pragma once

#include "Menu.h"
#include "Posture.h"

void Menu::Stamina()
{
  ImGuiMCP::Checkbox("Use Attack Stamina System", &Settings::bUseAttackStaminaSystem);
  ImGuiMCP::BeginTooltip();
  ImGuiMCP::TextWrapped("If disabled, stamina consumption will be the same as vanilla.");
  ImGuiMCP::EndTooltip();

  ImGuiMCP::Checkbox("Consume Stamina Out of Combat", &Settings::bConsumeStaminaOutCombat);
  ImGuiMCP::BeginTooltip();
  ImGuiMCP::SetTooltip("Whether stamina should be consumed when not in combat.");
  ImGuiMCP::EndTooltip();

  ImGuiMCP::Checkbox("Normal Attack Consume Stamina", &Settings::bNormalAttackComsumeStamina);
  ImGuiMCP::DragFloat("Normal Attack Stamina Cost Base", &Settings::fNormalAttackStaminaCostBase, 1.0f, 0.0f, 100.0f);
  ImGuiMCP::DragFloat("Normal Attack Stamina Cost Per Mass", &Settings::fNormalAttackStaminaCostPerMass, 0.01f, 0.0f, 10.0f);
  ImGuiMCP::DragFloat("Power Attack Stamina Cost Base", &Settings::fPowerAttackStaminaCostBase, 1.0f, 0.0f, 100.0f);
  ImGuiMCP::DragFloat("Power Attack Stamina Cost Per Mass", &Settings::fPowerAttackStaminaCostPerMass, 0.01f, 0.0f, 10.0f);

  ImGuiMCP::Separator();

  ImGuiMCP::DragFloat("Stamina Regen Minimum", &Settings::fStaminaRegenMin, 1.0f, 10.0f, Settings::fStaminaRegenLimit);
  ImGuiMCP::DragFloat("Stamina Regen Limit", &Settings::fStaminaRegenLimit, 1.0f, Settings::fStaminaRegenMin, 100.0f);
  ImGuiMCP::DragFloat("Stamina Regen Delay", &Settings::fStaminaRegenDelay, 1.0f, 0.1f, 5.0f);
  ImGuiMCP::DragFloat("Stamina Regen Multiplier (Combat)", &Settings::fStaminaRegenMultCombat, 0.1f, 0.1f, 1.0f);
}

void Menu::Posture()
{
  if (ImGuiMCP::Checkbox("Use Posture System", &Settings::bUsePostureSystem)) {
    if (!Settings::bUsePostureSystem) {
      Settings::bUsePostureHUD = false;
      Posture::GetSingleton().ReleaseHUD();
    } else {
      Settings::bUsePostureHUD = true;
      Posture::GetSingleton().InitHUD();
    }
  }
  if (ImGuiMCP::Checkbox("Use Posture HUD", &Settings::bUsePostureHUD)) {
    if (!Settings::bUsePostureHUD)
      Posture::GetSingleton().ReleaseHUD();
    else
      Posture::GetSingleton().InitHUD();
  }
  ImGuiMCP::DragFloat("Base Max Posture", &Settings::fMaxPostureBase, 1.0f, 50.0f, 200.0f);
  ImGuiMCP::DragFloat("Max Posture Health Multiplier", &Settings::fMaxPostureHealthMult, 0.01f, 0.0f, 1.0f);

  ImGuiMCP::DragFloat("Armor Resistance to Posture Damage Factor", &Settings::fArmorPostureDamageFactor, 0.1f, 0.1f, 10.0f);

  ImGuiMCP::Separator();
  ImGuiMCP::Checkbox("Enable Exhausted Feature", &Settings::bEnableExhausted);
  ImGuiMCP::DragFloat("Exhausted Restore Percent", &Settings::fExhaustedRestorePercent, 0.01f, 0.0f, 1.0f);
  ImGuiMCP::DragFloat("Exhausted Posture Damage Multiplier", &Settings::fExhaustedPostureDamageMult, 0.1f, 1.0f, 5.0f);
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
  if (!SKSEMenuFramework::IsInstalled())
    return;

  SKSEMenuFramework::SetSection("Rim Combat");

  SKSEMenuFramework::AddSectionItem("Stamina", Stamina);
  SKSEMenuFramework::AddSectionItem("Posture", Posture);

  // priority should be a individual value for each mod, here is nexus id of this mod
  event = new SKSEMenuFramework::Model::Event(EventListener, nexusID);

  logger::info("Menu: SKSEMenuFramework v{} loaded.", SKSEMenuFramework::GetMenuFrameworkVersion());
}