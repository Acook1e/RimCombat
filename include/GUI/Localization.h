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
Entry& GetLocalization(std::string key);  // 不推荐使用
bool AddLocalization(std::string key, std::string label, std::string desc);
}  // namespace Localization