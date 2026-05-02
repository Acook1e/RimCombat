#include "Combat/Exhausted.h"

#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "GUI/UI.h"
#include "Utils.h"

Exhausted::Exhausted()
{
  Serialization::RegisterSaveCallback(
      exhausted, [](SKSE::SerializationInterface* serial) {
        std::scoped_lock lock(mtx);
        // 将FormID转换为持久化格式
        // 并自动去除非法或未找到的FormID
        std::unordered_set<std::uint64_t> persistSet;
        for (const auto& formID : exhaustedActors) {
          auto persist = Serialization::ToPersistForm(formID);
          if (persist)
            persistSet.insert(persist);
        }

        std::uint32_t count = static_cast<std::uint32_t>(persistSet.size());
        serial->WriteRecordData(&count, sizeof(count));
        for (const auto& persist : persistSet) {
          serial->WriteRecordData(&persist, sizeof(persist));
        }
      });

  Serialization::RegisterLoadCallback(
      exhausted, [](SKSE::SerializationInterface* serial) {
        std::scoped_lock lock(mtx);
        exhaustedActors.clear();
        runtimeExhaustedActors.clear();

        std::uint32_t count;
        if (serial->ReadRecordData(&count, sizeof(count))) {
          for (std::uint32_t i = 0; i < count; ++i) {
            std::uint64_t persist;
            if (serial->ReadRecordData(&persist, sizeof(persist))) {
              auto formID = Serialization::ToForm(persist);
              if (formID)
                exhaustedActors.insert(formID);
            }
          }
        }
      });

  Serialization::RegisterRevertCallback(exhausted,
                                        [](SKSE::SerializationInterface*) {
                                          std::scoped_lock lock(mtx);
                                          exhaustedActors.clear();
                                          runtimeExhaustedActors.clear();
                                        });
}

bool Exhausted::IsActorExhausted(RE::Actor* actor)
{
  if (!actor)
    return false;
  std::scoped_lock lock(mtx);
  if (actor->GetActorBase()->IsUnique()) {
    return exhaustedActors.contains(actor->GetFormID());
  } else {
    return runtimeExhaustedActors.contains(actor);
  }
}

void Exhausted::EnterExhausted(RE::Actor* actor)
{
  if (!actor || !Settings::bEnableExhausted)
    return;
  std::scoped_lock lock(mtx);
  if (actor->GetActorBase()->IsUnique())
    exhaustedActors.insert(actor->GetFormID());
  else
    runtimeExhaustedActors.insert(actor);
  Utils::ActorCanAttack(actor, false);
  UI::TrueHUD::GetSingleton().EnterGreyOut(actor);
}

void Exhausted::ExitExhausted(RE::Actor* actor)
{
  if (!actor || !Settings::bEnableExhausted)
    return;
  std::scoped_lock lock(mtx);
  if (actor->GetActorBase()->IsUnique())
    exhaustedActors.erase(actor->GetFormID());
  else
    runtimeExhaustedActors.erase(actor);
  Utils::ActorCanAttack(actor, true);
  UI::TrueHUD::GetSingleton().ExitGreyOut(actor);
}