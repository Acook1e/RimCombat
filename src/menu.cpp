#include "menu.h"
#include "poiseHandler.h"

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

void Execution()
{
  ImGuiMCP::Text("Execution Settings go here.");
  if (ImGuiMCP::BeginCombo("Poise Type", Settings::PoiseTypeToString(Settings::poiseType).c_str())) {
    for (uint8_t i = PoiseType::kNull + 1; i < PoiseType::kTotal; i++) {
      PoiseType type  = static_cast<PoiseType>(i);
      bool isSelected = Settings::poiseType == type;
      if (ImGuiMCP::Selectable(Settings::PoiseTypeToString(type).c_str(), isSelected))
        Settings::poiseType = type;
      if (isSelected)
        ImGuiMCP::SetItemDefaultFocus();
    }
    ImGuiMCP::EndCombo();
  }
}

void Register()
{
  if (!SKSEMenuFramework::IsInstalled())
    return;

  SKSEMenuFramework::SetSection("Valhalla Combat Redux");

  SKSEMenuFramework::AddSectionItem("Block", Block);
  SKSEMenuFramework::AddSectionItem("Stamina", Stamina);
  SKSEMenuFramework::AddSectionItem("Execution", Execution);

  logger::info("Menu: Registered.");
}
}  // namespace Menu