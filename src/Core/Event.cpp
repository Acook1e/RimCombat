#include "Core/Event.h"

#include "Combat/Block.h"
#include "Combat/Stamina.h"
#include "Combat/WeaponArt.h"
#include "Core/Settings.h"
#include "GUI/UI.h"
#include "Utils.h"

namespace Events
{
// 返回True表示事件不需要往下传递了，返回False表示继续往下传递
bool AnimEvent::ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                             RE::BSAnimationGraphEvent* event,
                             RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource)
{
  if (!event->holder) {
    return false;
  }
  std::string eventTag = event->tag.data();
  std::transform(eventTag.begin(), eventTag.end(), eventTag.begin(), ::tolower);
  RE::Actor* actor = const_cast<RE::TESObjectREFR*>(event->holder)->As<RE::Actor>();

  // 过滤掉一些不必要的事件，减少日志噪音
  if (actor->IsPlayerRef() && eventTag != "scar_updatedummy" && eventTag != "pie" && false) {
    logger::info("Player Event: {}", eventTag);
  }
  switch (Utils::hash(eventTag.data(), eventTag.size())) {
  case "weaponswing"_h:
    if (Settings::bUseAttackStaminaSystem)
      Stamina::AttackStaminaConsume(actor, false);
    break;
  case "weaponleftswing"_h:
    if (Settings::bUseAttackStaminaSystem)
      Stamina::AttackStaminaConsume(actor, true);
    break;
  case "soundplay.wpnunarmedswing"_h:
    if (Settings::bUseAttackStaminaSystem)
      Stamina::AttackStaminaConsume(actor, false);
    break;
  case "attackstart"_h:
  case "mco_attackentry"_h:
  case "mco_powerattackentry"_h:
  case "bfco_playerattackstart"_h:
  case "bfco_npcattackstart"_h:
    if (Settings::bDisableAttackWhenStaminaZero &&
        actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) <= 0.0f) {
      SKSE::GetTaskInterface()->AddTask([actor]() {
        actor->NotifyAnimationGraph("attackStop");
      });
    }
    break;
  case "blockstart"_h:
  case "blockstartout"_h:
    Block::GetSingleton().StartBlock(actor);
    break;
  case "blockstop"_h:
    Block::GetSingleton().EndBlock(actor);
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

RE::BSEventNotifyControl
AnimEvent::ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                            RE::BSAnimationGraphEvent* event,
                            RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource)
{
  if (ProcessEvent(sink, event, eventSource))
    return RE::BSEventNotifyControl::kContinue;
  return _ProcessEvent_NPC(sink, event, eventSource);
}

RE::BSEventNotifyControl
AnimEvent::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                           RE::BSAnimationGraphEvent* event,
                           RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource)
{
  if (ProcessEvent(sink, event, eventSource))
    return RE::BSEventNotifyControl::kContinue;
  return _ProcessEvent_PC(sink, event, eventSource);
}

RE::BSEventNotifyControl
MenuEvent::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                        RE::BSTEventSource<RE::MenuOpenCloseEvent>* eventSource)
{
  if (event->menuName == RE::InventoryMenu::MENU_NAME) {
    if (event->opening)
      UI::WeaponArtMenu::SetInventoryMenuOpen(true);
    else
      UI::WeaponArtMenu::SetInventoryMenuOpen(false);
  }
  return RE::BSEventNotifyControl::kContinue;
}

void InputEvent::ProcessEvent(RE::BSTEventSource<RE::InputEvent*>* a_dispatcher,
                              RE::InputEvent* const* a_events)
{
  constexpr RE::InputEvent* const dummy[] = {nullptr};

  if (a_events && *a_events) {
    auto event = *a_events;
    if (event->GetEventType() == RE::INPUT_EVENT_TYPE::kButton) {
      auto buttonEvent = event->AsButtonEvent();
      if (buttonEvent && buttonEvent->IsDown()) {
        // Letter T
        if (UI::WeaponArtMenu::IsInventoryMenuShow() && buttonEvent->GetIDCode() == 20)
          UI::WeaponArtMenu::Toggle();
        // Left Alt key code
        else if (auto player = RE::PlayerCharacter::GetSingleton();
                 buttonEvent->GetIDCode() == 56 && player)
          WeaponArt::Manager::EnableWeaponArt(player, !WeaponArt::Manager::IsEnabled(player));
      }
    }
  }
  _ProcessEvent(a_dispatcher, a_events);
}
}  // namespace Events
