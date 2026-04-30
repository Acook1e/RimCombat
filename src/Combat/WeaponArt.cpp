#include "Combat/WeaponArt.h"

#include "Utils.h"

namespace WeaponArt
{
static std::unordered_map<WeaponArtType, std::string> typeToString;
std::string_view ToString(WeaponArtType type)
{
  auto it = typeToString.find(type);
  if (it != typeToString.end())
    return it->second;
  return "Unknown";
}

static std::unordered_map<WeaponArtRare, std::string> rareToString;
std::string_view ToString(WeaponArtRare rare)
{
  auto it = rareToString.find(rare);
  if (it != rareToString.end())
    return it->second;
  return "Unknown";
}

void Manager::Init()
{
  // Load weapon art info from files
  std::string weaponArtDir = std::string(Settings::SettingsDir) + "WeaponArt/";
}
}  // namespace WeaponArt
