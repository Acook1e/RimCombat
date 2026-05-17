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
  if (!event->holder)
    return false;

  std::string animEvent = event->tag.data();
  std::string payload   = event->payload.data();
  std::transform(animEvent.begin(), animEvent.end(), animEvent.begin(), ::tolower);
  std::transform(payload.begin(), payload.end(), payload.begin(), ::tolower);

  RE::Actor* actor = const_cast<RE::TESObjectREFR*>(event->holder)->As<RE::Actor>();

  switch (Utils::hash(animEvent)) {
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
  case "rimweaponart"_h: {
    if (payload == "start")
      actor->SetGraphVariableBool(WeaponArt::Manager::PERFORMING, true);
    else if (payload == "end")
      actor->SetGraphVariableBool(WeaponArt::Manager::PERFORMING, false);
    else if (payload == "prepareend")
      WeaponArt::Manager::SetEnabled(actor, true);
    else if (payload == "toprepare")
      WeaponArt::Manager::SetPrepare(actor, true);
    else if (payload.starts_with("stamina|"))
      Stamina::WeaponArtStaminaConsume(actor, payload.substr(8));
  } break;
  case "killactor"_h:
    // 如果进入处决状态，忽略KillMove的处决事件
    if (Execution::IsExecutingVictim(actor))
      return true;
    break;
  case "rimexecution"_h: {
    if (payload == "end")
      Execution::ExecutionEnd(actor);
    else if (payload.starts_with("damage|"))
      Execution::ApplyExecutionDamage(actor, payload.substr(7));
    // 等待拓展
  } break;
  case "staggerstart"_h:
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
    return RE::BSEventNotifyControl::kStop;
  return _ProcessEvent_NPC(sink, event, eventSource);
}

RE::BSEventNotifyControl
AnimEvent::ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                           RE::BSAnimationGraphEvent* event,
                           RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource)
{
  if (ProcessEvent(sink, event, eventSource))
    return RE::BSEventNotifyControl::kStop;
  return _ProcessEvent_PC(sink, event, eventSource);
}

RE::BSEventNotifyControl
MenuEvent::ProcessEvent(const RE::MenuOpenCloseEvent* event,
                        RE::BSTEventSource<RE::MenuOpenCloseEvent>* eventSource)
{
  auto ui = RE::UI::GetSingleton();
  if (!ui)
    return RE::BSEventNotifyControl::kContinue;

  // 监听物品栏菜单的开关来同步战技菜单的显示状态
  if (event->menuName == RE::InventoryMenu::MENU_NAME) {
    if (event->opening)
      UI::WeaponArtMenu::SetInventoryMenuOpen(true);
    else
      UI::WeaponArtMenu::SetInventoryMenuOpen(false);
  }

  bool showHUD = false;

  // 跟随HUD菜单的开关来显示/隐藏战技HUD
  if (ui->IsMenuOpen(RE::HUDMenu::MENU_NAME) || ui->IsMenuOpen("TrueHUD"))
    showHUD = true;

  if (ui->IsMenuOpen(RE::MainMenu::MENU_NAME) || ui->IsMenuOpen(RE::LoadingMenu::MENU_NAME) ||
      ui->IsMenuOpen(RE::InventoryMenu::MENU_NAME) || ui->IsMenuOpen(RE::MagicMenu::MENU_NAME) ||
      ui->IsMenuOpen(RE::BarterMenu::MENU_NAME) || ui->IsMenuOpen(RE::CraftingMenu::MENU_NAME) ||
      ui->IsMenuOpen(RE::Console::MENU_NAME) || ui->IsMenuOpen(RE::JournalMenu::MENU_NAME))
    showHUD = false;

  if (showHUD)
    UI::WeaponArtHUD::Show();
  else
    UI::WeaponArtHUD::Hide();

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
                 buttonEvent->GetIDCode() == 56 && player) {
          auto enabled  = WeaponArt::Manager::IsEnabled(player);
          auto prepared = WeaponArt::Manager::IsPrepared(player);
          WeaponArt::Manager::SwitchWeaponArt(player, !(enabled || prepared));
        }
      }
    }
  }
  _ProcessEvent(a_dispatcher, a_events);
}
}  // namespace Events
