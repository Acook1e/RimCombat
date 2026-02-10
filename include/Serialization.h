#pragma once

#include "Settings.h"

namespace SKSE
{
class SerializationInterface;
}

class Serialization
{
public:
  using SaveCallback   = std::function<void(SKSE::SerializationInterface*)>;
  using LoadCallback   = std::function<void(SKSE::SerializationInterface*, std::uint32_t, std::uint32_t)>;
  using RevertCallback = std::function<void(SKSE::SerializationInterface*)>;

  static Serialization& GetSingleton()
  {
    static Serialization singleton;
    return singleton;
  }

  void Init(const SKSE::SerializationInterface* a_interface);

  void RegisterRecordHandler(std::uint32_t a_type, std::uint32_t a_version, SaveCallback a_save, LoadCallback a_load);
  void RegisterRevertCallback(RevertCallback a_revert);

  void Save(SKSE::SerializationInterface* a_interface);
  void Load(SKSE::SerializationInterface* a_interface);
  void Revert(SKSE::SerializationInterface* a_interface);

private:
  struct RecordHandler
  {
    std::uint32_t type;
    std::uint32_t version;
    SaveCallback save;
    LoadCallback load;
  };

  static void SkipRecordData(const SKSE::SerializationInterface* a_interface, std::uint32_t a_length);

  std::vector<RecordHandler> recordHandlers;
  std::vector<RevertCallback> revertCallbacks;
};