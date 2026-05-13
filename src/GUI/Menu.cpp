#include "GUI/Menu.h"

#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "GUI/Localization.h"
#include "Utils.h"

#include "API/SKSEMenuFramework.h"

namespace ImGui
{
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
  if (!map.label.empty())
    ImGuiMCP::Text(std::vformat(map.label, std::make_format_args(args...)).data());
}
void Checkbox(std::uint32_t hash, bool* v, std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (ImGuiMCP::Checkbox(map.label.data(), v))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
void Button(std::uint32_t hash, std::function<void()> onClick = nullptr)
{
  const Localization::Entry& map = Localization::GetLocalization(hash);
  if (ImGuiMCP::Button(map.label.data()))
    if (onClick)
      onClick();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
}
void DragInt(std::uint32_t hash, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0,
             const char* format = "%d", std::function<void()> onChange = nullptr)
{
  const Localization::Entry& map   = Localization::GetLocalization(hash);
  ImGuiMCP::ImGuiSliderFlags flags = ImGuiMCP::ImGuiSliderFlags_None;
  if (ImGuiMCP::DragInt(map.label.data(), v, v_speed, v_min, v_max, format, flags))
    if (onChange)
      onChange();
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
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
  if (ImGuiMCP::IsItemHovered() && !map.desc.empty())
    ImGuiMCP::SetTooltip(map.desc.data());
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

void Menu::Debug()
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
}

SKSEMenuFramework::Model::Event* event = nullptr;

Menu::Menu()
{
  if (!SKSEMenuFramework::IsInstalled())
    return;

  ImGui::SetSection("RimCombat"_h);

  ImGui::AddSectionItem("Debug"_h, Debug);

  auto callback = [](SKSEMenuFramework::Model::EventType eventType) {
    switch (eventType) {
    case SKSEMenuFramework::Model::EventType::kOpenMenu:
      break;
    case SKSEMenuFramework::Model::EventType::kCloseMenu:
      Settings::SaveSettings();
      break;
    }
  };
  event = new SKSEMenuFramework::Model::Event(callback, static_cast<float>(MOD));

  logger::info("Menu: SKSEMenuFramework v{} loaded.", SKSEMenuFramework::GetMenuFrameworkVersion());
}

Menu::~Menu()
{
  if (event)
    delete event;
  event = nullptr;
}