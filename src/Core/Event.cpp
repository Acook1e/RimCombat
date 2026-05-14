#include "Core/Event.h"

#include "Combat/Block.h"
#include "Combat/Execution.h"
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

  switch (Utils::hash(eventTag)) {
  case "weaponswing"_h:
    Stamina::AttackStaminaConsume(actor, false);
    break;
  case "weaponleftswing"_h:
    Stamina::AttackStaminaConsume(actor, true);
    break;
  // 对于空手攻击，swing事件无法保证一定触发
  // 因此使用且仅使用weaponplay.wpnunarmedswing事件来检测空手攻击
  // 以确保在任何情况下都能正确消耗耐力
  case "soundplay.wpnunarmedswing"_h:
    Stamina::AttackStaminaConsume(actor, false, true);
    break;
  // 原版和MCO/BFCO框架下的攻击触发
  case "attackstart"_h:
  case "mco_attackentry"_h:
  case "mco_powerattackentry"_h:
  case "bfco_playerattackstart"_h:
  case "bfco_npcattackstart"_h:
    break;
  case "blockstart"_h:
  case "blockstartout"_h:
    Block::StartBlock(actor);
    break;
  case "blockstop"_h:
    Block::EndBlock(actor);
    break;
  case "rimcombat_executionend"_h:
    Execution::ExecutionEnd(actor);
    break;
  case "staggerstart"_h:
    // 用于在硬直触发后解除战技状态，暂时搁置
    // if (WeaponArt::Manager::IsEnabled(actor)) {
    //   auto artID = WeaponArt::Manager::GetActorWeaponArtID(actor);
    //   auto* art  = WeaponArt::Manager::GetWeaponArtInfo(artID);
    //   if (art && art->UseIntroAnim())
    //     WeaponArt::Manager::EnableWeaponArt(actor, false);
    // }
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
  // 监听物品栏菜单的开关来同步战技菜单的显示状态
  if (event->menuName == RE::InventoryMenu::MENU_NAME) {
    if (event->opening)
      UI::WeaponArtMenu::SetInventoryMenuOpen(true);
    else
      UI::WeaponArtMenu::SetInventoryMenuOpen(false);
  }

  // 跟随HUD菜单的开关来显示/隐藏战技HUD
  if (event->menuName == RE::HUDMenu::MENU_NAME || event->menuName == "TrueHUD") {
    if (event->opening)
      UI::WeaponArtHUD::Show();
    else
      UI::WeaponArtHUD::Hide();
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
