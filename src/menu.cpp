#include "menu.h"
#include "poiseHandler.h"
#include "setting.h"

#include "lib/SKSEMenuFramework.h"

namespace Menu
{
void Block()
{
  ImGuiMCP::Text("Block Settings go here.");
  ImGuiMCP::Checkbox("Enable Defence Damage Use Stamina", &Settings::bBlockStaminaToggle);
}

void Stamina()
{
  ImGuiMCP::Text("Stamina Settings go here.");
  ImGuiMCP::Checkbox("Consume Stamina Out of Combat", &Settings::bConsumeStaminaOutCombat);

  ImGuiMCP::Separator();
  ImGuiMCP::Checkbox("Light Attack Consume Stamina", &Settings::bNormalAttackComsumeStamina);

  ImGuiMCP::Separator();
  ImGuiMCP::Checkbox("Power Attack Consume Stamina", &Settings::bPowerAttackComsumeStaminaTweak);
}

void PoiseAndExecution()
{
  ImGuiMCP::Text("Poise and Execution Settings go here.");
  ImGuiMCP::Checkbox("Enable Poise and Execution System", &Settings::bEnablePoiseExecution);
  ImGuiMCP::Checkbox("Enable Player Auto Execution", &Settings::bEnablePlayerAutoExecution);
  ImGuiMCP::Checkbox("Enable NPC Auto Execution", &Settings::bEnableNPCAutoExecution);
  ImGuiMCP::SliderFloat("Execution Max Distance", &Settings::fExecutionMaxDistance, 50.0f, 300.0f, "%.1f");
  ImGuiMCP::Separator();
}

void Register()
{
  if (!SKSEMenuFramework::IsInstalled())
    return;

  SKSEMenuFramework::SetSection("Rim Combat");

  SKSEMenuFramework::AddSectionItem("Block", Block);
  SKSEMenuFramework::AddSectionItem("Stamina", Stamina);
  SKSEMenuFramework::AddSectionItem("Poise and Execution", PoiseAndExecution);

  logger::info("Menu: Registered.");
}
}  // namespace Menu