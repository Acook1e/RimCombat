#include "Combat/Exhausted.h"

#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "GUI/UI.h"
#include "Utils.h"

Exhausted::Exhausted()
{
  Serialization::RegisterSaveCallback(serialType, [](SKSE::SerializationInterface* serial) {
    std::scoped_lock lock(mtx);
    // 将FormID转换为持久化格式
    // 并自动去除非法或未找到的FormID
    std::unordered_set<std::uint64_t> persistSet;
    for (const auto* actor : exhaustedActors) {
      auto persist = Serialization::ToPersistForm(actor->GetFormID());
      if (persist)
        persistSet.insert(persist);
    }

    std::uint32_t count = static_cast<std::uint32_t>(persistSet.size());
    serial->WriteRecordData(&count, sizeof(count));
    for (const auto& persist : persistSet) {
      serial->WriteRecordData(&persist, sizeof(persist));
    }
  });

  Serialization::RegisterLoadCallback(serialType, [](SKSE::SerializationInterface* serial) {
    std::scoped_lock lock(mtx);
    exhaustedActors.clear();

    std::uint32_t count;
    if (serial->ReadRecordData(&count, sizeof(count))) {
      for (std::uint32_t i = 0; i < count; ++i) {
        std::uint64_t persist;
        if (serial->ReadRecordData(&persist, sizeof(persist))) {
          auto formID = Serialization::ToForm(persist);
          if (!formID)
            continue;
          auto form = RE::TESForm::LookupByID(formID);
          if (auto actor = form ? form->As<RE::Actor>() : nullptr; actor)
            exhaustedActors.insert(actor);
        }
      }
    }
  });

  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    std::scoped_lock lock(mtx);
    exhaustedActors.clear();
  });
}

bool Exhausted::IsActorExhausted(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExhaustedSystem)
    return false;
  std::scoped_lock lock(mtx);
  return exhaustedActors.contains(actor);
}

void Exhausted::EnterExhausted(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExhaustedSystem)
    return;
  std::scoped_lock lock(mtx);
  exhaustedActors.insert(actor);
  UI::TrueHUD::EnterGreyOut(actor);
}

void Exhausted::ExitExhausted(RE::Actor* actor)
{
  if (!actor || !Settings::bUseExhaustedSystem)
    return;
  std::scoped_lock lock(mtx);
  exhaustedActors.erase(actor);
  UI::TrueHUD::ExitGreyOut(actor);
}