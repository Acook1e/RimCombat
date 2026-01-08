#include "event.h"

#include "executionHandler.h"

namespace Events
{
constexpr uint32_t hash(const char* data, size_t const size) noexcept
{
  uint32_t hash = 114514;

  for (const char* c = data; c < data + size; ++c) {
    hash = ((hash << 5) + hash) + (unsigned char)*c;
  }

  return hash;
}

constexpr uint32_t operator""_h(const char* str, size_t size) noexcept
{
  return hash(str, size);
}

bool AnimEventHandler::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
                                    RE::BSAnimationGraphEvent* a_event,
                                    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (!a_event->holder) {
    return false;
  }
  std::string_view eventTag = a_event->tag.data();

  RE::Actor* actor = const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>();
  // if (Settings::bEnableAutoExecution && Settings::executorMark && actor->HasSpell(Settings::executorMark)) {
  //   logger::info("AnimEventHandler::ProcessEvent: Actor {} execution in progress, skipping event {}.",
  //                actor->GetDisplayFullName(), eventTag);
  // } else {
  // logger::info("AnimEventHandler::ProcessEvent: Actor {} event {}.", actor->GetDisplayFullName(), eventTag);
  // }
  switch (hash(eventTag.data(), eventTag.size())) {
  case "attackStart"_h:
  case "MCO_AttackEntry"_h:
  case "MCO_PowerAttackEntry"_h:
  case "BFCO_PlayerAttackStart"_h:
  case "BFCO_NPCAttackStart"_h:
    if ((Settings::bEnablePlayerAutoExecution && actor->IsPlayerRef()) ||
        (Settings::bEnableNPCAutoExecution && !actor->IsPlayerRef()))
      Handler::Execution::GetSingleton().TryExecution(actor);
    break;
  case "preHitFrame"_h:
    break;
  case "attackStop"_h:
    break;
  case "TKDR_IFrameEnd"_h:
    break;
  case "Dodge"_h:
    break;
  }
  return false;
}

RE::BSEventNotifyControl
AnimEventHandler::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
                                   RE::BSAnimationGraphEvent* a_event,
                                   RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (ProcessEvent(a_sink, a_event, a_eventSource))
    return RE::BSEventNotifyControl::kContinue;
  return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
}

RE::BSEventNotifyControl AnimEventHandler::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
                                                           RE::BSAnimationGraphEvent* a_event,
                                                           RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (ProcessEvent(a_sink, a_event, a_eventSource))
    return RE::BSEventNotifyControl::kContinue;
  return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
}
}  // namespace Events
