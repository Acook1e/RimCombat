#include "GUI/UI.h"

#include "Combat/Posture.h"
#include "Core/Settings.h"

#include "API/PrismaUI_API.h"
#include "API/TrueHUDAPI.h"

namespace UI
{
static PRISMA_UI_API::IVPrismaUI2* prisma = nullptr;
static TRUEHUD_API::IVTrueHUD4* truehud   = nullptr;

TrueHUD::TrueHUD()
{
  truehud = reinterpret_cast<TRUEHUD_API::IVTrueHUD4*>(
      TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V4));
  if (!truehud) {
    logger::info(
        "InitHUD: TrueHUD API init failed, TrueHUD maybe not installed?");
    return;
  }

  init = Require();
}

TrueHUD::~TrueHUD()
{
  Release();
  truehud = nullptr;
}

bool TrueHUD::Require()
{
  if (!truehud)
    return false;
  // 未启用架势HUD则不请求控制权
  if (!Settings::bUsePostureHUD)
    return false;
  auto res =
      truehud->RequestSpecialResourceBarsControl(SKSE::GetPluginHandle());
  switch (res) {
  case TRUEHUD_API::APIResult::OK:
  case TRUEHUD_API::APIResult::AlreadyGiven:
    logger::info("TrueHUD: Acquired HUD control.");
    truehud->RegisterSpecialResourceFunctions(
        SKSE::GetPluginHandle(), Posture::GetCurrentPosture,
        Posture::GetMaxPosture, true, true);
    return true;
  case TRUEHUD_API::APIResult::AlreadyTaken:
    logger::info("TrueHUD: HUD control already taken by another mod.");
    break;
  default:
    break;
  }
  return false;
}

bool TrueHUD::Release()
{
  if (!truehud)
    return false;
  // 如果未成功获取控制权，则无需释放
  if (!init)
    return true;
  if (truehud->ReleaseSpecialResourceBarControl(SKSE::GetPluginHandle()) ==
      TRUEHUD_API::APIResult::OK)
    return true;
  return false;
}

void TrueHUD::EnterGeryOut(RE::Actor* a_actor)
{
  if (!a_actor || !truehud || !init)
    return;
  truehud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina,
                            TRUEHUD_API::BarColorType::FlashColor, 0xd72a2a);
  truehud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina,
                            TRUEHUD_API::BarColorType::BarColor, 0x7d7e7d);
  truehud->OverrideBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina,
                            TRUEHUD_API::BarColorType::PhantomColor, 0xb30d10);
}

void TrueHUD::ExitGreyOut(RE::Actor* a_actor)
{
  if (!a_actor || !truehud || !init)
    return;
  truehud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina,
                          TRUEHUD_API::BarColorType::FlashColor);
  truehud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina,
                          TRUEHUD_API::BarColorType::BarColor);
  truehud->RevertBarColor(a_actor->GetHandle(), RE::ActorValue::kStamina,
                          TRUEHUD_API::BarColorType::PhantomColor);
}

}  // namespace UI