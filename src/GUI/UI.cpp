#include "GUI/UI.h"

#include "GUI/Localization.h"

#include "Combat/Posture.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"
#include "Utils.h"

#include "API/PrismaUI_API.h"
#include "API/TrueHUDAPI.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

namespace UI
{
using json = nlohmann::json;
using Localization::GetLocalizationLabel;

static PRISMA_UI_API::IVPrismaUI2* prisma = nullptr;
static TRUEHUD_API::IVTrueHUD4* truehud   = nullptr;

namespace
{
  constexpr std::uint64_t kWeaponArtMenuRefreshInterval = 200;

  template <typename E>
  std::string_view ResolveEnumLabel(E value, std::string_view fallback = "")
  {
    auto key = magic_enum::enum_name<E>(value);
    if (key.empty())
      return fallback;

    return GetLocalizationLabel(Utils::hash(key), key);
  }

  std::string_view ResolveWeaponArtStateLabel(WeaponArt::Manager::State state)
  {
    switch (state) {
    case WeaponArt::Manager::State::Disable:
      return GetLocalizationLabel("WeaponArtHUDStateDisable"_h, "Disabled");
    case WeaponArt::Manager::State::Prepare:
      return GetLocalizationLabel("WeaponArtHUDStatePrepare"_h, "Preparing");
    case WeaponArt::Manager::State::Enable:
      return GetLocalizationLabel("WeaponArtHUDStateEnable"_h, "Enabled");
    default:
      return GetLocalizationLabel("WeaponArtHUDStateDisable"_h, "Disabled");
    }
  }

  std::string_view ResolveWeaponArtName(std::int32_t artID)
  {
    if (artID == 0)
      return GetLocalizationLabel("WeaponArtUnassigned"_h, "Unassigned");

    if (auto* art = WeaponArt::Manager::GetWeaponArtInfo(artID))
      return art->GetName();

    return GetLocalizationLabel("WeaponArtUnknown"_h, "Unknown Weapon Art");
  }

  // 对于涉及到格式化的本地化文本，必须存在fallback
  // 否则有崩溃的风险
  json BuildWeaponArtMenuStrings()
  {
    return json{
        {"brand", GetLocalizationLabel("RimCombat"_h)},
        {"title", GetLocalizationLabel("WeaponArt"_h)},
        {"level", GetLocalizationLabel("WeaponArtMenuLevel"_h)},
        {"points", GetLocalizationLabel("WeaponArtMenuPoints"_h)},
        {"close", GetLocalizationLabel("WeaponArtMenuClose"_h)},
        {"catalog", GetLocalizationLabel("WeaponArtMenuCatalog"_h)},
        {"catalogTitle", GetLocalizationLabel("WeaponArtMenuCatalogTitle"_h)},
        {"catalogNote", GetLocalizationLabel("WeaponArtMenuCatalogNote"_h)},
        {"artCount", GetLocalizationLabel("WeaponArtMenuArtCount"_h, "{} Arts")},
        {"noDescription", GetLocalizationLabel("WeaponArtMenuNoDescription"_h)},
        {"noArtsTitle", GetLocalizationLabel("WeaponArtMenuNoArtsTitle"_h)},
        {"noArtsBody", GetLocalizationLabel("WeaponArtMenuNoArtsBody"_h)},
        {"currentWeapon", GetLocalizationLabel("WeaponArtMenuCurrentWeapon"_h)},
        {"noWeaponSelected", GetLocalizationLabel("WeaponArtMenuNoWeaponSelected"_h)},
        {"inventorySelectionRequired",
         GetLocalizationLabel("WeaponArtMenuInventorySelectionRequired"_h)},
        {"selectWeaponTitle", GetLocalizationLabel("WeaponArtMenuSelectWeaponTitle"_h)},
        {"selectWeaponBody", GetLocalizationLabel("WeaponArtMenuSelectWeaponBody"_h)},
        {"boundArt", GetLocalizationLabel("WeaponArtMenuBoundArt"_h)},
        {"weaponType", GetLocalizationLabel("WeaponArtMenuWeaponType"_h)},
        {"unassigned", GetLocalizationLabel("WeaponArtUnassigned"_h)},
        {"unknown", GetLocalizationLabel("Unknown"_h)},
        {"weaponBody", GetLocalizationLabel("WeaponArtMenuWeaponBody"_h)},
        {"noWeaponArtSelected", GetLocalizationLabel("WeaponArtMenuNoWeaponArtSelected"_h)},
        {"selectedArtAlreadyBound", GetLocalizationLabel("WeaponArtMenuSelectedArtAlreadyBound"_h)},
        {"bindAction", GetLocalizationLabel("WeaponArtMenuBindAction"_h, "Bind {}")},
        {"unbindCurrent", GetLocalizationLabel("WeaponArtMenuUnbindCurrent"_h)},
        {"noWeaponArtBound", GetLocalizationLabel("WeaponArtMenuNoWeaponArtBound"_h)},
        {"selectedArt", GetLocalizationLabel("WeaponArtMenuSelectedArt"_h)},
        {"noArtSelected", GetLocalizationLabel("WeaponArtMenuNoArtSelected"_h)},
        {"waitingTitle", GetLocalizationLabel("WeaponArtMenuWaitingTitle"_h)},
        {"waitingBody", GetLocalizationLabel("WeaponArtMenuWaitingBody"_h)},
        {"unlockLevel", GetLocalizationLabel("WeaponArtMenuUnlockLevel"_h)},
        {"pointCost", GetLocalizationLabel("WeaponArtMenuPointCost"_h)},
        {"activation", GetLocalizationLabel("WeaponArtMenuActivation"_h)},
        {"assignment", GetLocalizationLabel("WeaponArtMenuAssignment"_h)},
        {"available", GetLocalizationLabel("WeaponArtMenuAvailable"_h)},
        {"currentlyAssigned", GetLocalizationLabel("WeaponArtMenuCurrentlyAssigned"_h)},
        {"unlocked", GetLocalizationLabel("WeaponArtMenuUnlocked"_h)},
        {"locked", GetLocalizationLabel("WeaponArtMenuLocked"_h)},
        {"levelBadge", GetLocalizationLabel("WeaponArtMenuLevelBadge"_h, "Lv {}")},
        {"costBadge", GetLocalizationLabel("WeaponArtMenuCostBadge"_h, "Cost {}")},
        {"prepare", GetLocalizationLabel("WeaponArtMenuPrepare"_h)},
        {"instant", GetLocalizationLabel("WeaponArtMenuInstant"_h)},
        {"compatible", GetLocalizationLabel("WeaponArtMenuCompatible"_h)},
        {"incompatible", GetLocalizationLabel("WeaponArtMenuIncompatible"_h)},
        {"assigned", GetLocalizationLabel("WeaponArtMenuAssigned"_h)},
        {"alreadyUnlocked", GetLocalizationLabel("WeaponArtMenuAlreadyUnlocked"_h)},
        {"unlockAction", GetLocalizationLabel("WeaponArtMenuUnlockAction"_h, "Unlock Art ({} pt)")},
        {"assignedToWeapon", GetLocalizationLabel("WeaponArtMenuAssignedToWeapon"_h)},
        {"assignToSelectedWeapon", GetLocalizationLabel("WeaponArtMenuAssignToSelectedWeapon"_h)},
        {"hintSelectArt", GetLocalizationLabel("WeaponArtMenuHintSelectArt"_h)},
        {"hintCanBind",
         GetLocalizationLabel("WeaponArtMenuHintCanBind"_h,
                              "Bind {} to this weapon, or clear the current binding.")},
        {"hintHasBinding", GetLocalizationLabel("WeaponArtMenuHintHasBinding"_h)},
        {"hintNoBinding", GetLocalizationLabel("WeaponArtMenuHintNoBinding"_h)},
        {"hintUnlockBlocked", GetLocalizationLabel("WeaponArtMenuHintUnlockBlocked"_h,
                                                   "Requires level {} and {} available point(s).")},
        {"hintNeedWeapon", GetLocalizationLabel("WeaponArtMenuHintNeedWeapon"_h)},
        {"hintIncompatible", GetLocalizationLabel("WeaponArtMenuHintIncompatible"_h)},
        {"hintAlreadyAssigned", GetLocalizationLabel("WeaponArtMenuHintAlreadyAssigned"_h)},
        {"hintReadyToAssign", GetLocalizationLabel("WeaponArtMenuHintReadyToAssign"_h)},
        {"hintUnlockFirst", GetLocalizationLabel("WeaponArtMenuHintUnlockFirst"_h)},
        {"hintStateUpdated", GetLocalizationLabel("WeaponArtMenuHintStateUpdated"_h)}};
  }

  json BuildWeaponArtHUDStrings()
  {
    return json{{"label", GetLocalizationLabel("WeaponArtHUDLabel"_h)},
                {"defaultName", GetLocalizationLabel("WeaponArtUnassigned"_h)},
                {"disabled", GetLocalizationLabel("WeaponArtHUDStateDisable"_h)},
                {"preparing", GetLocalizationLabel("WeaponArtHUDStatePrepare"_h)},
                {"enabled", GetLocalizationLabel("WeaponArtHUDStateEnable"_h)}};
  }

  std::optional<std::int32_t> ParseArtID(const char* arg)
  {
    if (!arg || !*arg)
      return std::nullopt;

    char* end     = nullptr;
    auto rawValue = std::strtol(arg, &end, 10);
    if (end == arg || *end != '\0')
      return std::nullopt;
    if (rawValue < (std::numeric_limits<std::int32_t>::min)() ||
        rawValue > (std::numeric_limits<std::int32_t>::max)())
      return std::nullopt;

    return static_cast<std::int32_t>(rawValue);
  }
}  // namespace

void Initialize()
{
  truehud = reinterpret_cast<TRUEHUD_API::IVTrueHUD4*>(
      TRUEHUD_API::RequestPluginAPI(TRUEHUD_API::InterfaceVersion::V4));
  if (!truehud) {
    logger::info("UI::Initialize: TrueHUD API init failed, TrueHUD maybe not "
                 "installed?");
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
  auto res = truehud->RequestSpecialResourceBarsControl(SKSE::GetPluginHandle());
  switch (res) {
  case TRUEHUD_API::APIResult::OK:
  case TRUEHUD_API::APIResult::AlreadyGiven:
    logger::info("TrueHUD: Acquired HUD control.");
    truehud->RegisterSpecialResourceFunctions(SKSE::GetPluginHandle(), Posture::GetCurrentPosture,
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
  view = prisma->CreateView("RimCombat_WeaponArtMenu/menu.html", OnViewReady);

  if (!prisma->IsValid(view)) {
    logger::info("UI: WeaponArtMenu view creation failed.");
    return;
  }

  prisma->Hide(view);
  prisma->RegisterJSListener(view, "closeWeaponArtMenu", [](const char*) {
    Hide();
  });
  prisma->RegisterJSListener(view, "unlockWeaponArt", UnlockWeaponArt);
  prisma->RegisterJSListener(view, "setWeaponArt", SetWeaponArt);
  prisma->RegisterConsoleCallback(view, nullptr);
}

WeaponArtMenu::~WeaponArtMenu()
{
  if (prisma && prisma->IsValid(view)) {
    prisma->Unfocus(view);
    prisma->Destroy(view);
  }

  view = 0;
}

RE::FormID WeaponArtMenu::GetSelectedObjectFormID()
{
  auto* itemEntry = Utils::GetSelectedItemEntry();
  if (!itemEntry || !itemEntry->object)
    return 0;

  return itemEntry->object->GetFormID();
}

RE::TESObjectWEAP* WeaponArtMenu::GetSelectedWeapon()
{
  auto* itemEntry = Utils::GetSelectedItemEntry();
  if (!itemEntry)
    return nullptr;

  auto* object = itemEntry->object;
  if (!object)
    return nullptr;
  return object->IsWeapon() ? object->As<RE::TESObjectWEAP>() : nullptr;
}

void WeaponArtMenu::OnViewReady(PrismaView readyView)
{
  domReady = true;
  logger::info("UI: WeaponArtMenu DOM ready");

  if (!isShow && prisma && prisma->IsValid(readyView))
    prisma->Hide(readyView);

  SyncViewConfig();

  if (isShow)
    SyncViewData();
}

void WeaponArtMenu::SyncViewConfig()
{
  if (!prisma || !prisma->IsValid(view) || !domReady)
    return;

  json payload = {{"startPercent", Settings::fWeaponArtMenuStartPercent},
                  {"strings", BuildWeaponArtMenuStrings()}};

  auto data = payload.dump();
  prisma->InteropCall(view, "setMenuConfig", data.c_str());
}

void WeaponArtMenu::SyncViewData()
{
  if (!prisma || !prisma->IsValid(view) || !domReady)
    return;

  json payload;
  payload["playerLevel"]   = WeaponArt::PlayerStat::GetLevel();
  payload["playerPoint"]   = WeaponArt::PlayerStat::GetPoint();
  payload["filterEnabled"] = false;

  auto* selectedWeapon      = GetSelectedWeapon();
  std::int32_t currentArtID = 0;
  if (selectedWeapon) {
    currentArtID     = WeaponArt::Manager::GetWeaponArtID(selectedWeapon);
    auto* weaponName = selectedWeapon->GetName();

    payload["selectedWeapon"] = {
        {"name", weaponName ? weaponName : ""},
        {"type", ResolveEnumLabel(Weapon::GetWeaponType(selectedWeapon),
                                  GetLocalizationLabel("UnknownWeaponType"_h, "Unknown"))},
        {"currentArtId", currentArtID},
        {"currentArtName", ResolveWeaponArtName(currentArtID)}};
  } else {
    payload["selectedWeapon"] = nullptr;
  }

  auto arts = json::array();
  for (const auto* art : WeaponArt::Manager::GetAllWeaponArts()) {
    if (!WeaponArt::PlayerStat::IsOwned(art->GetID()))
      continue;

    auto unlocked      = WeaponArt::PlayerStat::IsUnlocked(art->GetID());
    auto weaponAllowed = selectedWeapon && art->IsWeaponAllowed(selectedWeapon);

    arts.push_back({{"id", art->GetID()},
                    {"name", art->GetName()},
                    {"description", art->GetDescription()},
                    {"consumePoint", art->GetConsumePoint()},
                    {"unlockLevel", art->GetUnlockLevel()},
                    {"needPrepare", art->NeedPrepare()},
                    {"unlocked", unlocked},
                    {"weaponAllowed", weaponAllowed},
                    {"isAssigned", selectedWeapon && currentArtID == art->GetID()}});
  }

  payload["arts"] = std::move(arts);

  auto data = payload.dump();
  prisma->InteropCall(view, "setMenuState", data.c_str());
}

void WeaponArtMenu::Update()
{
  if (!prisma || !prisma->IsValid(view) || !domReady || !isShow || !inventoryMenuShow)
    return;

  const auto now = Utils::GetTime<std::chrono::milliseconds>();
  if (now - lastSyncTime < kWeaponArtMenuRefreshInterval)
    return;

  lastSyncTime = now;

  const auto selectedObjectFormID = GetSelectedObjectFormID();
  if (selectedObjectFormID == lastSelectedObjectFormID)
    return;

  lastSelectedObjectFormID = selectedObjectFormID;
  SyncViewData();
}

void WeaponArtMenu::Toggle()
{
  if (isShow)
    Hide();
  else
    Show();
}

void WeaponArtMenu::Show()
{
  if (!prisma || !prisma->IsValid(view) || !inventoryMenuShow)
    return;

  isShow                   = true;
  lastSyncTime             = 0;
  lastSelectedObjectFormID = static_cast<RE::FormID>(-1);
  prisma->Show(view);
  prisma->Focus(view);
  SyncViewConfig();
  if (domReady)
    SyncViewData();
}

void WeaponArtMenu::Hide()
{
  if (!prisma || !prisma->IsValid(view))
    return;

  isShow                   = false;
  lastSyncTime             = 0;
  lastSelectedObjectFormID = 0;
  prisma->Unfocus(view);
  prisma->Hide(view);
}

void WeaponArtMenu::SetInventoryMenuOpen(bool open)
{
  inventoryMenuShow = open;
  if (!open && isShow)
    Hide();
}

void WeaponArtMenu::UnlockWeaponArt(const char* arg)
{
  auto id = ParseArtID(arg);
  if (!id)
    return;

  WeaponArt::PlayerStat::UnlockArt(id.value());
  SyncViewData();
}

void WeaponArtMenu::SetWeaponArt(const char* arg)
{
  auto id = ParseArtID(arg);
  if (!id)
    return;

  auto* weapon = GetSelectedWeapon();
  if (!weapon)
    return;

  WeaponArt::Manager::SetWeaponArtInfo(weapon, id.value());
  SyncViewData();
}

WeaponArtHUD::WeaponArtHUD()
{
  if (!prisma)
    return;
  view = prisma->CreateView("RimCombat_WeaponArtHUD/hud.html", OnViewReady);

  if (!prisma->IsValid(view)) {
    logger::info("UI: WeaponArtHUD view creation failed.");
    return;
  }

  prisma->Hide(view);
  prisma->RegisterConsoleCallback(view, nullptr);
}

WeaponArtHUD::~WeaponArtHUD()
{
  if (prisma && prisma->IsValid(view)) {
    prisma->Unfocus(view);
    prisma->Destroy(view);
  }

  view = 0;
}

void WeaponArtHUD::OnViewReady(PrismaView readyView)
{
  domReady = true;
  logger::info("UI: WeaponArtHUD DOM ready");

  if (!isShow && prisma && prisma->IsValid(readyView))
    prisma->Hide(readyView);

  SyncViewConfig();
}

void WeaponArtHUD::Show()
{
  if (!prisma || !prisma->IsValid(view) || !Settings::bUseWeaponArtHUD || !canShow)
    return;

  isShow = true;
  prisma->Show(view);
  SyncViewConfig();

  UpdateName(WeaponArt::Manager::GetActorWeaponArtID(RE::PlayerCharacter::GetSingleton()));
  auto state = WeaponArt::Manager::GetState(RE::PlayerCharacter::GetSingleton());
  UpdateState(state);
}

void WeaponArtHUD::Hide()
{
  if (!prisma || !prisma->IsValid(view))
    return;

  isShow = false;
  prisma->Hide(view);
}

void WeaponArtHUD::UpdateState(WeaponArt::Manager::State state)
{
  if (!prisma || !prisma->IsValid(view) || !domReady || !isShow)
    return;

  json payload = {{"state", static_cast<std::uint8_t>(state)},
                  {"text", ResolveWeaponArtStateLabel(state)}};

  auto data = payload.dump();
  prisma->InteropCall(view, "setHudState", data.c_str());
}

void WeaponArtHUD::UpdateName(std::int32_t artID)
{
  if (!prisma || !prisma->IsValid(view) || !domReady || !isShow)
    return;

  json payload = {{"name", ResolveWeaponArtName(artID)}};

  auto data = payload.dump();
  prisma->InteropCall(view, "setHudName", data.c_str());
}

void WeaponArtHUD::SyncViewConfig()
{
  if (!prisma || !prisma->IsValid(view) || !domReady)
    return;

  json payload = {{"x", Settings::fWeaponArtHUDPosX},
                  {"y", Settings::fWeaponArtHUDPosY},
                  {"scale", Settings::fWeaponArtHUDScale},
                  {"strings", BuildWeaponArtHUDStrings()},
                  {"label", GetLocalizationLabel("WeaponArtHUDLabel"_h, "Weapon Art")},
                  {"defaultName", GetLocalizationLabel("WeaponArtUnassigned"_h, "Unassigned")}};

  auto data = payload.dump();
  prisma->InteropCall(view, "setHudConfig", data.c_str());
}
}  // namespace UI