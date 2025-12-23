#pragma once

#include "pch.h"

#include "blockHandler.h"
#include "poiseHandler.h"

#include "lib/TrueHUDAPI.h"

namespace Handler
{
class UpdateHandler
{
public:
  static UpdateHandler& GetSingleton()
  {
    static UpdateHandler singleton;
    return singleton;
  }

  void InitHUD()
  {
    hud = reinterpret_cast<TRUEHUD_API::IVTrueHUD3*>(TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V3));
    if (hud) {
      logger::info("UpdateHandler: TrueHUD API Installed.");
    } else {
      logger::info("UpdateHandler: TrueHUD API not found.");
    }
  }

  void Update() {}

  void RequestHUD()
  {
    if (!hud)
      logger::info("UpdateHandler: HUD interface not found.");

    auto res = hud->RequestSpecialResourceBarsControl(SKSE::GetPluginHandle());
    switch (res) {
    case TRUEHUD_API::APIResult::OK:
    case TRUEHUD_API::APIResult::AlreadyGiven:
      logger::info("UpdateHandler: Acquired HUD control.");
      hud->RegisterSpecialResourceFunctions(SKSE::GetPluginHandle(), PoiseHandler::GetCurrentPoise,
                                            PoiseHandler::GetMaxPoise, true, true);
      Settings::bHudEnabled = true;
      break;
    case TRUEHUD_API::APIResult::AlreadyTaken:
      logger::info("UpdateHandler: HUD control already taken by another mod.");
      break;
    }
  }

  void ReleaseHUD()
  {
    if (!hud)
      return;

    auto res = hud->ReleaseSpecialResourceBarControl(SKSE::GetPluginHandle());
    switch (res) {
    case TRUEHUD_API::APIResult::OK:
      logger::info("UpdateHandler: Released HUD control.");
      Settings::bHudEnabled = false;
      break;
    case TRUEHUD_API::APIResult::MustKeep:
      logger::info("UpdateHandler: Must keep HUD control.");
      break;
    case TRUEHUD_API::APIResult::NotOwner:
      logger::info("UpdateHandler: Not owner of HUD control.");
      break;
    }
  }

private:
  TRUEHUD_API::IVTrueHUD3* hud;
};
}  // namespace Handler