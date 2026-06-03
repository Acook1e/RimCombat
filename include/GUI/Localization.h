#pragma once

#include "Core/Settings.h"

namespace Localization
{

const inline std::string LocalizationFile = Settings::SettingsDir + "Localization.json";

struct Entry
{
  std::string label;
  std::string desc;
};

void Initialize();
Entry& GetLocalization(std::uint32_t hash);
std::string_view GetLocalizationLabel(std::uint32_t hash, std::string_view fallback = "");
std::string_view GetLocalizationDesc(std::uint32_t hash, std::string_view fallback = "");
Entry& GetLocalization(std::string key);  // 不推荐使用
bool AddLocalization(std::string key, std::string label, std::string desc);
}  // namespace Localization