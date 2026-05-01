#include "Core/Serialization.h"
#include "Utils.h"

namespace Serialization
{
static std::unordered_map<std::uint32_t, Callback> saveCallbacks;
static std::unordered_map<std::uint32_t, Callback> loadCallbacks;
static std::unordered_map<std::uint32_t, Callback> revertCallbacks;

constexpr static std::uint32_t SerializationVersion = 1;

void Initialize()
{
  auto* serial = SKSE::GetSerializationInterface();
  serial->SetUniqueID(MOD);
  serial->SetSaveCallback([](SKSE::SerializationInterface* serial) {
    for (const auto& [type, callback] : saveCallbacks)
      if (serial->OpenRecord(type, SerializationVersion))
        callback(serial);
      else
        logger::error(
            "Serialization::Initialize: Failed to open record for type: {}",
            type);
  });
  serial->SetLoadCallback([](SKSE::SerializationInterface* serial) {
    std::uint32_t type, version, length;
    while (serial->GetNextRecordInfo(type, version, length)) {
      if (version != SerializationVersion) {
        logger::error("Serialization::Initialize: Unsupported serialization "
                      "version: {} for "
                      "type: {}, expected: {}. Skipping {} bytes",
                      version, type, SerializationVersion, length);
        continue;
      }

      auto it = loadCallbacks.find(type);
      if (it != loadCallbacks.end())
        it->second(serial);
      else
        logger::warn("Serialization::Initialize: No load callback registered "
                     "for type: {}, "
                     "skipping {} bytes",
                     type, length);
    }
  });
  serial->SetRevertCallback([](SKSE::SerializationInterface* serial) {
    for (const auto& [type, callback] : revertCallbacks)
      callback(serial);
  });
}

std::uint32_t GetVersion()
{
  return SerializationVersion;
}

bool RegisterSaveCallback(std::uint32_t type, Callback callback)
{
  if (saveCallbacks.find(type) != saveCallbacks.end())
    return false;  // Type already registered
  return saveCallbacks.emplace(type, std::move(callback)).second;
}

bool RegisterLoadCallback(std::uint32_t type, Callback callback)
{
  if (loadCallbacks.find(type) != loadCallbacks.end())
    return false;  // Type already registered
  return loadCallbacks.emplace(type, std::move(callback)).second;
}

bool RegisterRevertCallback(std::uint32_t type, Callback callback)
{
  if (revertCallbacks.find(type) != revertCallbacks.end())
    return false;  // Type already registered
  return revertCallbacks.emplace(type, std::move(callback)).second;
}

std::uint64_t ToPersistForm(RE::FormID formID)
{
  if (formID >= 0xFF000000)
    return 0;  // Not a mod form, cannot persist
  auto* dataHandler = RE::TESDataHandler::GetSingleton();
  if (!dataHandler)
    return 0;
  const RE::TESFile* mod = nullptr;
  std::uint32_t rawForm  = 0;
  if (formID >= 0xFE000000) {
    mod = dataHandler->LookupLoadedLightModByIndex((formID & 0x00FFF000) >> 12);
    rawForm = formID & 0x00000FFF;
  } else {
    mod     = dataHandler->LookupLoadedModByIndex(formID >> 24);
    rawForm = formID & 0x00FFFFFF;
  }
  if (!mod || !rawForm)
    return 0;  // Mod not found or invalid formID
  return (static_cast<std::uint64_t>(Utils::hash(mod->fileName)) << 32) |
         rawForm;
}

RE::FormID ToForm(std::uint64_t persistForm)
{
  static std::unordered_map<std::uint32_t, std::uint32_t> modHashToIndexCache;

  std::uint32_t modHash = persistForm >> 32;
  std::uint32_t rawForm = persistForm & 0xFFFFFFFF;
  if (auto it = modHashToIndexCache.find(modHash);
      it != modHashToIndexCache.end()) {
    return it->second | rawForm;
  } else {
    auto* dataHandler = RE::TESDataHandler::GetSingleton();
    if (!dataHandler)
      return 0;
    auto* mods = dataHandler->GetLoadedMods();
    for (std::size_t i = 0; i < dataHandler->GetLoadedModCount(); ++i) {
      auto* mod = mods[i];
      if (mod->compileIndex && mod->compileIndex != 0xFF)
        modHashToIndexCache[Utils::hash(mod->fileName)] = mod->compileIndex
                                                          << 24;
    }
    mods = dataHandler->GetLoadedLightMods();
    for (std::size_t i = 0; i < dataHandler->GetLoadedLightModCount(); ++i) {
      auto* mod = mods[i];
      if (mod->smallFileCompileIndex)
        modHashToIndexCache[Utils::hash(mod->fileName)] =
            (mod->smallFileCompileIndex << 12) | 0xFE000000;
    }
    if (auto it = modHashToIndexCache.find(modHash);
        it != modHashToIndexCache.end())
      return it->second | rawForm;
    return 0;  // Not found
  }
}
}  // namespace Serialization