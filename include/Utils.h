#pragma once

namespace Utils
{
// 时间辅助函数，默认精度为毫秒
template <typename T>
[[nodiscard]] std::uint64_t
GetTime(T accuracy = std::chrono::milliseconds()) noexcept
{
  return std::chrono::duration_cast<T>(
             std::chrono::steady_clock::now().time_since_epoch())
      .count();
}

// 计时辅助类，在析构时输出从创建到析构的时间，单位为毫秒
class ScopeTimer
{
public:
  ScopeTimer(std::string_view name)
      : name(name), start(std::chrono::high_resolution_clock::now())
  {}

  ~ScopeTimer()
  {
    auto end = std::chrono::high_resolution_clock::now();
    auto duration =
        std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
            .count();
    logger::info("{} took {} ms", name, duration);
  }

private:
  std::string_view name;
  std::chrono::high_resolution_clock::time_point start;
};

// 字符串工具
std::string join(std::vector<std::string>& vec, char delimiter);
std::vector<std::string> split(const std::string& str, char delimiter);

// 哈希相关函数
constexpr inline std::uint32_t hash(const char* data,
                                    size_t const size) noexcept
{
  uint32_t hash = MOD;
  for (const char* c = data; c < data + size; ++c) {
    hash = ((hash << 5) + hash) + (unsigned char)*c;
  }
  return hash;
}
constexpr inline std::uint32_t hash(std::string_view str) noexcept
{
  return hash(str.data(), str.size());
}

// 游戏相关工具
RE::InventoryEntryData* GetSelectedItemEntry();
float GetCurrentMaxActorValue(RE::Actor* actor, RE::ActorValue av);
void ActorCanAttack(RE::Actor* actor, bool enable);  // 仅对NPC有效
template <typename T>
void SetGameSettings(const char* setting, T value)
{
  RE::Setting* gameSetting = nullptr;
  RE::GameSettingCollection* _settingCollection =
      RE::GameSettingCollection::GetSingleton();
  gameSetting = _settingCollection->GetSetting(setting);
  if (!gameSetting) {
    logger::info("SetGameSetting: Invalid setting: {}", setting);
  } else {
    if constexpr (std::is_same_v<T, bool>) {
      gameSetting->data.b = value;
    } else if constexpr (std::is_same_v<T, float>) {
      gameSetting->data.f = value;
    } else if constexpr (std::is_same_v<T, std::int32_t>) {
      gameSetting->data.i = value;
    } else if constexpr (std::is_same_v<T, RE::Color>) {
      gameSetting->data.r = value;
    } else if constexpr (std::is_same_v<T, char*>) {
      gameSetting->data.s = value;
    } else if constexpr (std::is_same_v<T, std::uint32_t>) {
      gameSetting->data.u = value;
    }
  }
}
}  // namespace Utils

// 显式使用Utils命名空间中的ScopeTimer和hash函数
using ScopeTimer = Utils::ScopeTimer;
constexpr std::uint32_t operator""_h(const char* str, size_t size) noexcept
{
  return Utils::hash(str, size);
}