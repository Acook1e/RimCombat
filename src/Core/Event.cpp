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
#include "GUI/Localization.h"
#include "GUI/UI.h"
#include "Utils.h"

#include "API/InputManagerAPI.h"


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
  // 对于空手攻击，swing事件无法保证一定触发
  // 因此使用且仅使用weaponplay.wpnunarmedswing事件来检测空手攻击
  // 以确保在任何情况下都能正确消耗耐力
  case "soundplay.wpnunarmedswing"_h:
    Stamina::UnarmStaminaConsume(actor);
    break;
  case "blockstart"_h:
  case "blockstartout"_h:
    Block::StartBlock(actor);
    break;
  case "blockstop"_h:
    Block::EndBlock(actor);
    break;
  case "attackstart"_h:
    // 如果攻击开始但战技还是已经进行中的状态，说明是End事件触发太晚
    // 此时强制中断当前战技状态
    if (WeaponArt::Manager::GetPerform(actor) != WeaponArt::Manager::Perform::None)
      WeaponArt::Manager::End(actor);
    break;
  // MCO/BFCO框架下的攻击触发
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
    Posture::TargetEnd(actor);
    Stagger::TargetEnd(actor);
    Stamina::End(actor);
    WeaponArt::Manager::Interrupt(actor);
    break;

  case "killactor"_h:
    // 如果进入处决状态，忽略KillMove的处决事件
    if (Execution::IsExecutingVictim(actor))
      return true;
    break;
  case "staggerstop"_h: {
    auto currentLevel = Stagger::GetStaggerLevel(actor);
    if (currentLevel != Stagger::Level::None)
      break;
    auto recordLevel = Stagger::IsInStagger(actor);
    if (recordLevel != Stagger::Level::None)
      Stagger::Recoverable(actor);
    break;
  }
  case "prehitframe"_h:
    break;
  case "tkdr_iframeend"_h:
    break;
  case "dodge"_h:
    break;

    // 进入拔刀状态
  case "beginweapondraw"_h:
    if (Settings::bHideWeaponArtHUDOnSheathe && actor->IsPlayerRef())
      UI::WeaponArtHUD::Show();
    break;

    // 收刀状态
  case "beginweaponsheathe"_h:
    if (Settings::bHideWeaponArtHUDOnSheathe && actor->IsPlayerRef())
      UI::WeaponArtHUD::Hide();
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

  auto player = RE::PlayerCharacter::GetSingleton();
  if (Settings::bHideWeaponArtHUDOnSheathe && !player->AsActorState()->IsWeaponDrawn())
    showHUD = false;

  if (showHUD)
    UI::WeaponArtHUD::Show();
  else
    UI::WeaponArtHUD::Hide();

  return RE::BSEventNotifyControl::kContinue;
}

void ModEvent::RegisterWeaponArtHUDInput()
{
  if (!InputManagerAPI::_API)
    return;

  auto* API = InputManagerAPI::_API;

  constexpr std::int32_t inputType     = 0;
  constexpr std::string_view inputName = "SwitchWeaponArtState";
  const auto& purpose = Localization::GetLocalization("SwitchWeaponArtState_Purpose");

  auto inputId = API->CreateInput(inputType, inputName.data());
  if (inputId == -1) {
    auto size = API->GetInputCount(inputType);
    for (std::size_t i = 0; i < size; ++i)
      if (API->GetInputName(inputType, i) == inputName)
        inputId = i;
  } else {
    // 左Alt按键 按下一次
    const InputManagerAPI::ActionInfo defaultInfo = {inputId, inputName.data(), 56, 1, 1};
    API->UpdateActionMapping(inputId, defaultInfo);
  }
  API->UpdateListener(inputType, inputId, PLUGIN_NAME.data(), purpose.label.data(), true);
  id_WeaponArtHUD = inputId;
}
void ModEvent::RegisterWeaponArtMenuInput()
{
  if (!InputManagerAPI::_API)
    return;

  auto* API = InputManagerAPI::_API;

  constexpr std::int32_t inputType     = 0;
  constexpr std::string_view inputName = "SwitchWeaponArtMenu";
  const auto& purpose = Localization::GetLocalization("SwitchWeaponArtMenu_Purpose");

  auto inputId = API->CreateInput(inputType, inputName.data());
  if (inputId == -1) {
    auto size = API->GetInputCount(inputType);
    for (std::size_t i = 0; i < size; ++i)
      if (API->GetInputName(inputType, i) == inputName)
        inputId = i;
  } else {
    // T按键 按下一次
    const InputManagerAPI::ActionInfo defaultInfo = {inputId, inputName.data(), 20, 1, 1};
    API->UpdateActionMapping(inputId, defaultInfo);
  }
  API->UpdateListener(inputType, inputId, PLUGIN_NAME.data(), purpose.label.data(), true);
  id_WeaponArtMenu = inputId;
}
RE::BSEventNotifyControl
ModEvent::ProcessEvent(const SKSE::ModCallbackEvent* event,
                       RE::BSTEventSource<SKSE::ModCallbackEvent>* eventSource)
{
  if (!event || !eventSource)
    return RE::BSEventNotifyControl::kContinue;

  auto trigger = event->eventName == "InputManager_ActionTriggered";
  auto release = event->eventName == "InputManager_ActionReleased";

  if (!trigger && !release)
    return RE::BSEventNotifyControl::kContinue;

  auto inputID = static_cast<std::int32_t>(event->numArg);
  if (inputID == -1)
    return RE::BSEventNotifyControl::kContinue;

  if (inputID == id_WeaponArtHUD) {
    // 此时可以保证API不为空指针
    auto* API   = InputManagerAPI::_API;
    auto action = API->GetActionInfo(inputID);

    // 检测是否是按住模式
    auto hold = action.pcMainAction == 2 || action.gamepadMainAction == 2;

    auto player = RE::PlayerCharacter::GetSingleton();
    if (player) {
      if (!hold) {
        // 切换模式下的按键行为：按一下切换一次状态，松开不做反应
        auto state = WeaponArt::Manager::GetState(player);
        WeaponArt::Manager::SwitchWeaponArt(player, state == WeaponArt::Manager::State::Disable);
      } else {
        // 按住模式下的按键行为：按住切换到开启状态，松开切换到关闭状态
        if (trigger)
          WeaponArt::Manager::SwitchWeaponArt(player, true);
        else if (release)
          WeaponArt::Manager::SwitchWeaponArt(player, false);
      }
    }
  } else if (inputID == id_WeaponArtMenu) {
    if (UI::WeaponArtMenu::IsInventoryMenuShow())
      UI::WeaponArtMenu::Toggle();
  }

  return RE::BSEventNotifyControl::kContinue;
}
}  // namespace Events
