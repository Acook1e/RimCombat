#include "event.h"

namespace Events
{
constexpr uint32_t hash(const char* data, size_t const size) noexcept
{
  uint32_t hash = 5381;

  for (const char* c = data; c < data + size; ++c) {
    hash = ((hash << 5) + hash) + (unsigned char)*c;
  }

  return hash;
}

constexpr uint32_t operator""_h(const char* str, size_t size) noexcept
{
  return hash(str, size);
}

void animEventHandler::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
                                    RE::BSAnimationGraphEvent* a_event,
                                    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (!a_event->holder) {
    return;
  }
  std::string_view eventTag = a_event->tag.data();

  switch (hash(eventTag.data(), eventTag.size())) {
  case "preHitFrame"_h:

    break;
  case "attackStop"_h:

    break;
  // case "blockStop"_h:
  //	if (settings::bTimedBlockToggle && a_event->holder->IsPlayerRef()) {
  //		blockHandler::GetSingleton()->onBlockStop();
  //	}
  //	break;
  case "TKDR_IFrameEnd"_h:

    break;
  case "Dodge"_h:

    break;
  }
}

RE::BSEventNotifyControl
animEventHandler::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
                                   RE::BSAnimationGraphEvent* a_event,
                                   RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  ProcessEvent(a_sink, a_event, a_eventSource);
  return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
}

RE::BSEventNotifyControl animEventHandler::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink,
                                                           RE::BSAnimationGraphEvent* a_event,
                                                           RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  ProcessEvent(a_sink, a_event, a_eventSource);
  return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
}
}  // namespace Events