#include "Serialization.h"

void Serialization::Init(const SKSE::SerializationInterface* a_interface)
{
  if (!a_interface) {
    logger::error("Serialization: interface is null.");
    return;
  }

  a_interface->SetUniqueID(nexusID);
  a_interface->SetSaveCallback([](SKSE::SerializationInterface* a_interface) {
    GetSingleton().Save(a_interface);
  });
  a_interface->SetLoadCallback([](SKSE::SerializationInterface* a_interface) {
    GetSingleton().Load(a_interface);
  });
  a_interface->SetRevertCallback([](SKSE::SerializationInterface* a_interface) {
    GetSingleton().Revert(a_interface);
  });

  logger::info("Serialization: callbacks registered.");
}

void Serialization::RegisterRecordHandler(std::uint32_t a_type, std::uint32_t a_version, SaveCallback a_save, LoadCallback a_load)
{
  recordHandlers.push_back({a_type, a_version, std::move(a_save), std::move(a_load)});
}

void Serialization::RegisterRevertCallback(RevertCallback a_revert)
{
  revertCallbacks.push_back(std::move(a_revert));
}

void Serialization::Save(SKSE::SerializationInterface* a_interface)
{
  if (!a_interface) {
    logger::error("Serialization::Save: interface is null.");
    return;
  }

  for (const auto& handler : recordHandlers) {
    if (!handler.save) {
      continue;
    }
    if (!a_interface->OpenRecord(handler.type, handler.version)) {
      logger::error("Serialization::Save: failed to open record type {:X}.", handler.type);
      continue;
    }
    handler.save(a_interface);
  }
}

void Serialization::Load(SKSE::SerializationInterface* a_interface)
{
  if (!a_interface) {
    logger::error("Serialization::Load: interface is null.");
    return;
  }

  std::uint32_t type    = 0;
  std::uint32_t version = 0;
  std::uint32_t length  = 0;

  while (a_interface->GetNextRecordInfo(type, version, length)) {
    auto it = std::find_if(recordHandlers.begin(), recordHandlers.end(), [type](const RecordHandler& handler) {
      return handler.type == type;
    });

    if (it == recordHandlers.end() || !it->load) {
      logger::warn("Serialization::Load: unknown record type {:X}, skipping.", type);
      SkipRecordData(a_interface, length);
      continue;
    }

    if (version != it->version) {
      logger::warn("Serialization::Load: record type {:X} version mismatch. save={}, load={}", type, version, it->version);
    }

    it->load(a_interface, version, length);
  }
}

void Serialization::Revert(SKSE::SerializationInterface* a_interface)
{
  for (const auto& callback : revertCallbacks) {
    if (callback) {
      callback(a_interface);
    }
  }
}

void Serialization::SkipRecordData(const SKSE::SerializationInterface* a_interface, std::uint32_t a_length)
{
  constexpr std::uint32_t kChunkSize = 4096;
  std::array<std::uint8_t, kChunkSize> buffer{};
  std::uint32_t remaining = a_length;

  while (remaining > 0) {
    const auto toRead = std::min(remaining, kChunkSize);
    const auto read   = a_interface->ReadRecordData(buffer.data(), toRead);
    if (read == 0) {
      break;
    }
    remaining -= read;
  }
}