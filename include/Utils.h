#pragma once

namespace Utils
{
template <typename T>
void SetGameSettings(const char* a_setting, T a_value)
{
  RE::Setting* setting                          = nullptr;
  RE::GameSettingCollection* _settingCollection = RE::GameSettingCollection::GetSingleton();
  setting                                       = _settingCollection->GetSetting(a_setting);
  if (!setting) {
    logger::info("SetGameSetting: Invalid setting: {}", a_setting);
  } else {
    if constexpr (std::is_same_v<T, bool>) {
      setting->data.b = a_value;
    } else if constexpr (std::is_same_v<T, float>) {
      setting->data.f = a_value;
    } else if constexpr (std::is_same_v<T, std::int32_t>) {
      setting->data.i = a_value;
    } else if constexpr (std::is_same_v<T, RE::Color>) {
      setting->data.r = a_value;
    } else if constexpr (std::is_same_v<T, char*>) {
      setting->data.s = a_value;
    } else if constexpr (std::is_same_v<T, std::uint32_t>) {
      setting->data.u = a_value;
    }
  }
}

float GetCurrentMaxActorValue(RE::Actor* actor, RE::ActorValue av);

template <typename T>
std::int64_t GetTime(T accuracy)
{
  return std::chrono::duration_cast<T>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

constexpr std::uint32_t hash(const char* data, size_t const size) noexcept
{
  uint32_t hash = nexusID;

  for (const char* c = data; c < data + size; ++c) {
    hash = ((hash << 5) + hash) + (unsigned char)*c;
  }

  return hash;
}
constexpr std::uint32_t operator""_h(const char* str, size_t size) noexcept
{
  return hash(str, size);
}
}  // namespace Utils