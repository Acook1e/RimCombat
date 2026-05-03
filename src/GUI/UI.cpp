#include "GUI/UI.h"

#include "Combat/Posture.h"
#include "Core/Settings.h"

#include "API/PrismaUI_API.h"
#include "API/TrueHUDAPI.h"

namespace UI
{
static PRISMA_UI_API::IVPrismaUI2* prisma = nullptr;
static TRUEHUD_API::IVTrueHUD4* truehud   = nullptr;

void Initialize()
{
  truehud = reinterpret_cast<TRUEHUD_API::IVTrueHUD4*>(
      TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V4));
  if (!truehud) {
    logger::info("UI::Initialize: TrueHUD API init failed, TrueHUD maybe not "
                 "installed?");
    return;
  }

  prisma = reinterpret_cast<PRISMA_UI_API::IVPrismaUI2*>(
      PRISMA_UI_API::RequestPluginAPI(PRISMA_UI_API::InterfaceVersion::V2));
  if (!prisma) {
    logger::info("UI::Initialize: PrismaUI API init failed, PrismaUI maybe not "
                 "installed?");
  }
}

TrueHUD::TrueHUD()
{
  init = Require();
}

TrueHUD::~TrueHUD()
{
  Release();
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

void TrueHUD::EnterGreyOut(RE::Actor* actor)
{
  if (!actor || !truehud || !init)
    return;
  truehud->OverrideBarColor(actor->GetHandle(), RE::ActorValue::kStamina,
                            TRUEHUD_API::BarColorType::FlashColor, 0xd72a2a);
  truehud->OverrideBarColor(actor->GetHandle(), RE::ActorValue::kStamina,
                            TRUEHUD_API::BarColorType::BarColor, 0x7d7e7d);
  truehud->OverrideBarColor(actor->GetHandle(), RE::ActorValue::kStamina,
                            TRUEHUD_API::BarColorType::PhantomColor, 0xb30d10);
}

void TrueHUD::ExitGreyOut(RE::Actor* actor)
{
  if (!actor || !truehud || !init)
    return;
  truehud->RevertBarColor(actor->GetHandle(), RE::ActorValue::kStamina,
                          TRUEHUD_API::BarColorType::FlashColor);
  truehud->RevertBarColor(actor->GetHandle(), RE::ActorValue::kStamina,
                          TRUEHUD_API::BarColorType::BarColor);
  truehud->RevertBarColor(actor->GetHandle(), RE::ActorValue::kStamina,
                          TRUEHUD_API::BarColorType::PhantomColor);
}

WeaponArtMenu::WeaponArtMenu()
{
  if (!prisma)
    return;
  view =
      prisma->CreateView("RimCombat_WeaponArtMenu/menu.html", [](PrismaView) {
        logger::info("UI: WeaponArtMenu view closed.");
      });

  if (!prisma->IsValid(view)) {
    logger::info("UI: WeaponArtMenu view creation failed.");
    return;
  }

  prisma->RegisterJSListener(view, "unlockWeaponArt", UnlockWeaponArt);
  prisma->RegisterJSListener(view, "setWeaponArt", SetWeaponArt);
}

WeaponArtMenu::~WeaponArtMenu()
{
  if (prisma && prisma->IsValid(view)) {
    prisma->Unfocus(view);
    prisma->Destroy(view);
  }

  view = 0;
}

void WeaponArtMenu::Show()
{
  if (!prisma || !prisma->IsValid(view) || inventoryMenuOpen)
    return;

  // 发送数据

  isOpen = true;
  prisma->Show(view);
}

void WeaponArtMenu::Hide()
{
  if (!prisma || !prisma->IsValid(view))
    return;

  isOpen = false;
  prisma->Unfocus(view);
  prisma->Hide(view);
}

}  // namespace UI