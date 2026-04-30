#pragma once

#include "magic_enum/magic_enum.hpp"

namespace Localization
{
struct Entry
{
  std::string label;
  std::string desc;
};

void Initialize();
Entry& GetLocalization(std::uint32_t hash);
bool AddLocalization(std::string key, std::string label, std::string desc);

template <typename E>
std::vector<std::reference_wrapper<Entry>> BuildEnumLocalization()
{
  const auto& enum_name   = magic_enum::enum_type_name<E>();
  const auto& value_names = magic_enum::enum_names<E>();
  std::vector<std::reference_wrapper<Entry>> maps;
  for (const auto& name : value_names)
    maps.push_back(std::ref(GetLocalization(hash(name))));
  return maps;
}
}  // namespace Localization