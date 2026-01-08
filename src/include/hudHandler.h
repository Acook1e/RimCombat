#pragma once

#include "pch.h"

#include "poiseHandler.h"
#include "setting.h"

#include "lib/TrueHUDAPI.h"

namespace Handler
{
class HUD
{
public:
  static HUD& GetSingleton()
  {
    static HUD singleton;
    return singleton;
  }

  void InitHUD()
  {
    hud = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
    if (hud) {
      logger::info("Handler::HUD: TrueHUD API Installed.");
    } else {
      logger::info("Handler::HUD: TrueHUD API not found.");
    }
  }

  void RequestHUD()
  {
    if (!hud)
      logger::info("Handler::HUD: HUD interface not found.");

    auto res = hud->RequestSpecialResourceBarsControl(SKSE::GetPluginHandle());
    switch (res) {
    case TRUEHUD_API::APIResult::OK:
    case TRUEHUD_API::APIResult::AlreadyGiven:
      logger::info("Handler::HUD: Acquired HUD control.");
      hud->RegisterSpecialResourceFunctions(SKSE::GetPluginHandle(), Poise::GetCurrentPoise, Poise::GetMaxPoise, true,
                                            true);
      Settings::bHudEnabled = true;
      break;
    case TRUEHUD_API::APIResult::AlreadyTaken:
      logger::info("Handler::HUD: HUD control already taken by another mod.");
      break;
    }
  }

  bool ReleaseHUD()
  {
    if (!hud)
      return false;

    if (hud->ReleaseSpecialResourceBarControl(SKSE::GetPluginHandle()) == TRUEHUD_API::APIResult::OK) {
      Settings::bHudEnabled = false;
      return true;
    }
    return false;
  }

  void EnterExhaustedHUD(RE::Actor* a_actor)
  {
    if (!hud || !Settings::bHudEnabled)
      return;

    hud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::FlashColor,
                          0xd72a2a);
    hud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::BarColor,
                          0x7d7e7d);
    hud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::PhantomColor,
                          0xb30d10);
  }

  void QuitExhaustedHUD(RE::Actor* a_actor)
  {
    if (!hud || !Settings::bHudEnabled)
      return;
    hud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::FlashColor);
    hud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::BarColor);
    hud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina, TRUEHUD_API::BarColorType::PhantomColor);
  }

private:
  TRUEHUD_API::IVTrueHUD3* hud;
};
}  // namespace Handler