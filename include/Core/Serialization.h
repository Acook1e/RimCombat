#pragma once

namespace Serialization
{
using Callback = std::function<void(SKSE::SerializationInterface*)>;

void Initialize();
std::uint32_t GetVersion();
bool RegisterSaveCallback(std::uint32_t type, Callback callback);
bool RegisterLoadCallback(std::uint32_t type, Callback callback);
bool RegisterRevertCallback(std::uint32_t type, Callback callback);

// 持久化FormID转换，避免因Mod加载顺序变化导致的FormID变化问题
std::uint64_t ToPersistForm(RE::FormID formID);
// 从持久化格式转换回FormID，如果Mod未加载或FormID无效则返回0
RE::FormID ToForm(std::uint64_t persistForm);
}  // namespace Serialization