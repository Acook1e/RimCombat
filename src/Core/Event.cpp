#include "Core/Event.h"

#include "Combat/Block.h"
#include "Combat/Damage.h"
#include "Combat/Execution.h"
#include "Combat/Poise.h"
#include "Combat/Posture.h"
#include "Combat/Stagger.h"
#include "Combat/Stamina.h"
#include "Combat/WeaponArt.h"
#include "Core/Settings.h"
#include "GUI/UI.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"

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
    Stamina::SwingStaminaConsume(actor, false);
    break;
  case "weaponleftswing"_h:
    Stamina::SwingStaminaConsume(actor, true);
    break;
  // 对于空手攻击，swing事件无法保证一定触发
  // 因此使用且仅使用weaponplay.wpnunarmedswing事件来检测空手攻击
  // 以确保在任何情况下都能正确消耗耐力
  case "soundplay.wpnunarmedswing"_h:
    Stamina::SwingStaminaConsume(actor, false, true);
    break;
  case "collision_attackstart"_h:
    Stamina::PrecisionStart(actor);
    break;
  case "collision_attackend"_h:
    Stamina::PrecisionEnd(actor);
    break;
  case "collision_add"_h:
    Stamina::CollisionStaminaConsume(actor, payload);
    break;
  case "blockstart"_h:
  case "blockstartout"_h:
    Block::StartBlock(actor);
    break;
  case "blockstop"_h:
    Block::EndBlock(actor);
    break;
  // 原版和MCO/BFCO框架下的攻击触发
  case "attackstart"_h:
  case "mco_attackentry"_h:
  case "mco_powerattackentry"_h:
  case "bfco_playerattackstart"_h:
  case "bfco_npcattackstart"_h:
    break;

  // 攻击停止或者触发硬直重置只用于一次攻击的系统
  case "attackstop"_h:
  case "staggerstart"_h:
    Damage::End(actor);
    Poise::End(actor);
    Poise::TargetEnd(actor);
    Posture::End(actor);
    Stagger::TargetEnd(actor);
    Stamina::End(actor);
    Stamina::PrecisionEnd(actor);
    WeaponArt::Manager::Interrupt(actor);
    break;

  case "killactor"_h:
    // 如果进入处决状态，忽略KillMove的处决事件
    if (Execution::IsExecutingVictim(actor))
      return true;
    break;
  case "staggerstop"_h: {
    // 如果在可恢复状态后解除硬直，则硬直生命周期结束
    auto recoverable = false;
    if (actor->GetGraphVariableBool(Stagger::STAGGER_RECOVERABLE, recoverable))
      Stagger::SetStaggerLevel(actor, Stagger::Level::None);
    break;
  }
  case "prehitframe"_h:
    break;
  case "tkdr_iframeend"_h:
    break;
  case "dodge"_h:
    break;
  case "rimblock"_h:
    Block::ParsePayload(actor, payload);
    break;
  case "rimdamage"_h:
    Damage::PayloadParse(actor, payload);
    break;
  case "rimexecution"_h:
    Execution::PayloadParse(actor, payload);
    break;
  case "rimstagger"_h:
    Stagger::PayloadParse(actor, payload);
    break;
  case "rimstamina"_h:
    Stamina::PayloadParse(actor, payload);
    break;
  case "rimpoise"_h:
    Poise::PayloadParse(actor, payload);
    break;
  case "rimposture"_h:
    Posture::PayloadParse(actor, payload);
    break;
  case "rimweaponart"_h:
    WeaponArt::Manager::PayloadParse(actor, payload);
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

RE::BSEventNotifyControl HitEvent::ProcessEvent(const RE::TESHitEvent* event,
                                                RE::BSTEventSource<RE::TESHitEvent>* eventSource)
{
  if (!event || !event->target || !event->source || !event->projectile)
    return RE::BSEventNotifyControl::kContinue;

  auto victim = event->target->As<RE::Actor>();
  Stagger::ProcessProjectileStagger(victim, event->source);

  return RE::BSEventNotifyControl::kContinue;
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
          auto state = WeaponArt::Manager::GetState(player);
          WeaponArt::Manager::SwitchWeaponArt(player, state == WeaponArt::Manager::State::Disable);
        }
      }
    }
  }
  _ProcessEvent(a_dispatcher, a_events);
}
}  // namespace Events
