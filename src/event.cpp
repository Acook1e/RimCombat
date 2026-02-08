#include "Event.h"

#include "Stamina.h"
#include "Utils.h"

namespace Events
{

using Utils::operator""_h;
bool AnimEvent::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                             RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (!a_event->holder) {
    return false;
  }
  std::string eventTag = a_event->tag.data();
  std::transform(eventTag.begin(), eventTag.end(), eventTag.begin(), ::tolower);
  RE::Actor* actor = const_cast<RE::TESObjectREFR*>(a_event->holder)->As<RE::Actor>();
  //   if (actor->IsPlayerRef() && eventTag != "scar_updatedummy") {
  //     logger::info("Player Event: {}", eventTag);
  //   }
  switch (Utils::hash(eventTag.data(), eventTag.size())) {
  case "weaponswing"_h:
  case "weaponleftswing"_h:
    if (Settings::bUseAttackStaminaSystem)
      Stamina::AttackStaminaConsume(actor, false, true);
    break;
  case "soundplay.wpnunarmedswing"_h:
    if (Settings::bUseAttackStaminaSystem)
      Stamina::AttackStaminaConsume(actor, false, true, true);
    break;
  case "attackstart"_h:
  case "mco_attackentry"_h:
  case "mco_powerattackentry"_h:
  case "bfco_playerattackstart"_h:
  case "bfco_npcattackstart"_h:
    break;
  case "prehitframe"_h:
    break;
  case "attackstop"_h:
    break;
  case "tkdr_iframeend"_h:
    break;
  case "dodge"_h:
    break;
  }
  return false;
}

RE::BSEventNotifyControl AnimEvent::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                     RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (ProcessEvent(a_sink, a_event, a_eventSource))
    return RE::BSEventNotifyControl::kContinue;
  return _ProcessEvent_NPC(a_sink, a_event, a_eventSource);
}

RE::BSEventNotifyControl AnimEvent::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* a_sink, RE::BSAnimationGraphEvent* a_event,
                                                    RE::BSTEventSource<RE::BSAnimationGraphEvent>* a_eventSource)
{
  if (ProcessEvent(a_sink, a_event, a_eventSource))
    return RE::BSEventNotifyControl::kContinue;
  return _ProcessEvent_PC(a_sink, a_event, a_eventSource);
}
}  // namespace Events
