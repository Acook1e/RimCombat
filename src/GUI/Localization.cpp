#include "GUI/Localization.h"

#include "Core/Settings.h"
#include "Utils.h"

#include "nlohmann/json.hpp"

namespace Localization
{
static std::unordered_map<std::uint32_t, Entry> entryMaps;

void Initialize()
{
  nlohmann::json j;
  try {
    std::ifstream ifs(LocalizationFile);
    j = nlohmann::json::parse(ifs);
  } catch (const std::exception& e) {
    logger::error("Localization::Initialize: Failed to load localization file: "
                  "{}. Error: {}",
                  LocalizationFile, e.what());
    return;
  }

  if (j.is_null() || !j.is_object()) {
    logger::error("Localization::Initialize: Localization file is not a valid "
                  "JSON object.");
    return;
  }

  entryMaps.reserve(j.size());
  for (const auto& [key, value] : j.items()) {
    if (value.empty() || !value.is_object())
      continue;

    auto label = value.value("label", "");
    auto desc  = value.value("desc", "");

    entryMaps[Utils::hash(key)] = {label, desc};
  }
}

Entry& GetLocalization(std::uint32_t hash)
{
  static Entry invalid = {"Invalid", "Invalid"};
  auto it              = entryMaps.find(hash);
  return (it != entryMaps.end()) ? it->second : invalid;
}

std::string_view GetLocalizationLabel(std::uint32_t hash, std::string_view fallback)
{
  auto it = entryMaps.find(hash);
  return (it != entryMaps.end()) ? it->second.label : fallback;
}

std::string_view GetLocalizationDesc(std::uint32_t hash, std::string_view fallback)
{
  auto it = entryMaps.find(hash);
  return (it != entryMaps.end()) ? it->second.desc : fallback;
}

Entry& GetLocalization(std::string key)
{
  return GetLocalization(Utils::hash(key));
}

bool AddLocalization(std::string key, std::string label, std::string desc)
{
  auto hashValue = Utils::hash(key);
  if (entryMaps.find(hashValue) != entryMaps.end()) {
    logger::warn("Localization::AddLocalization: Localization key '{}' already "
                 "exists. Skipping.",
                 key);
    return false;
  }
  entryMaps[hashValue] = {std::move(label), std::move(desc)};
  return true;
}
}  // namespace Localization