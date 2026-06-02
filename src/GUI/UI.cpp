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

static PRISMA_UI_API::IVPrismaUI2* prisma = nullptr;
static TRUEHUD_API::IVTrueHUD4* truehud   = nullptr;

namespace
{
  constexpr std::uint64_t kWeaponArtMenuRefreshInterval = 200;

  const Localization::Entry& ResolveEntry(std::string_view key)
  {
    static const Localization::Entry emptyEntry{};
    if (key.empty())
      return emptyEntry;

    return Localization::GetLocalization(Utils::hash(std::string(key)));
  }

  std::string ResolveLabel(std::string_view key, std::string_view fallback = "")
  {
    if (key.empty())
      return std::string(fallback);

    auto& entry = ResolveEntry(key);
    if (!entry.label.empty() && entry.label != "null")
      return entry.label;

    return fallback.empty() ? std::string(key) : std::string(fallback);
  }

  template <class Enum>
  std::string ResolveEnumLabel(Enum value, std::string_view fallback = "")
  {
    auto key = magic_enum::enum_name(value);
    if (key.empty())
      return std::string(fallback);

    return ResolveLabel(key, key);
  }

  std::string ResolveWeaponArtStateLabel(WeaponArt::Manager::State state)
  {
    switch (state) {
    case WeaponArt::Manager::State::Disable:
      return ResolveLabel("WeaponArtHUDStateDisable", "Disabled");
    case WeaponArt::Manager::State::Prepare:
      return ResolveLabel("WeaponArtHUDStatePrepare", "Preparing");
    case WeaponArt::Manager::State::Enable:
      return ResolveLabel("WeaponArtHUDStateEnable", "Enabled");
    default:
      return ResolveLabel("WeaponArtHUDStateDisable", "Disabled");
    }
  }

  std::string ResolveWeaponArtName(std::int32_t artID)
  {
    if (artID == 0)
      return ResolveLabel("WeaponArtUnassigned", "Unassigned");

    if (auto* art = WeaponArt::Manager::GetWeaponArtInfo(artID))
      return art->GetName();

    return ResolveLabel("WeaponArtUnknown", "Unknown Weapon Art");
  }

  json BuildWeaponArtMenuStrings()
  {
    return json{
        {"brand", ResolveLabel("RimCombat", "RimCombat")},
        {"title", ResolveLabel("WeaponArt", "Weapon Arts")},
        {"level", ResolveLabel("WeaponArtMenuLevel", "Weapon Art Level")},
        {"points", ResolveLabel("WeaponArtMenuPoints", "Weapon Art Points")},
        {"close", ResolveLabel("WeaponArtMenuClose", "Close")},
        {"catalog", ResolveLabel("WeaponArtMenuCatalog", "Catalog")},
        {"catalogTitle", ResolveLabel("WeaponArtMenuCatalogTitle", "All Weapon Arts")},
        {"catalogNote", ResolveLabel("WeaponArtMenuCatalogNote",
                                     "Select a weapon art below. The upper panels show the "
                                     "selected weapon and the highlighted art.")},
        {"artCount", ResolveLabel("WeaponArtMenuArtCount", "{} Arts")},
        {"noDescription", ResolveLabel("WeaponArtMenuNoDescription", "No description available.")},
        {"noArtsTitle", ResolveLabel("WeaponArtMenuNoArtsTitle", "No weapon arts loaded")},
        {"noArtsBody",
         ResolveLabel(
             "WeaponArtMenuNoArtsBody",
             "The menu is ready, but no weapon art data was received from the plugin yet.")},
        {"currentWeapon", ResolveLabel("WeaponArtMenuCurrentWeapon", "Current Weapon")},
        {"noWeaponSelected", ResolveLabel("WeaponArtMenuNoWeaponSelected", "No weapon selected")},
        {"inventorySelectionRequired",
         ResolveLabel("WeaponArtMenuInventorySelectionRequired", "Inventory selection required")},
        {"selectWeaponTitle",
         ResolveLabel("WeaponArtMenuSelectWeaponTitle", "Select a weapon in the inventory")},
        {"selectWeaponBody", ResolveLabel("WeaponArtMenuSelectWeaponBody",
                                          "The upper-left panel shows the selected weapon, its "
                                          "equipped art, and the bind state.")},
        {"boundArt", ResolveLabel("WeaponArtMenuBoundArt", "Bound Art")},
        {"weaponType", ResolveLabel("WeaponArtMenuWeaponType", "Weapon Type")},
        {"unassigned", ResolveLabel("WeaponArtUnassigned", "Unassigned")},
        {"unknown", ResolveLabel("Unknown", "Unknown")},
        {"weaponBody",
         ResolveLabel("WeaponArtMenuWeaponBody", "The highlighted weapon art can be bound to this "
                                                 "weapon if it is unlocked and compatible.")},
        {"noWeaponArtSelected",
         ResolveLabel("WeaponArtMenuNoWeaponArtSelected", "No Weapon Art Selected")},
        {"selectedArtAlreadyBound",
         ResolveLabel("WeaponArtMenuSelectedArtAlreadyBound", "Selected Art Already Bound")},
        {"bindAction", ResolveLabel("WeaponArtMenuBindAction", "Bind {}")},
        {"unbindCurrent", ResolveLabel("WeaponArtMenuUnbindCurrent", "Unbind Current Weapon Art")},
        {"noWeaponArtBound", ResolveLabel("WeaponArtMenuNoWeaponArtBound", "No Weapon Art Bound")},
        {"selectedArt", ResolveLabel("WeaponArtMenuSelectedArt", "Selected Art")},
        {"noArtSelected", ResolveLabel("WeaponArtMenuNoArtSelected", "No art selected")},
        {"waitingTitle", ResolveLabel("WeaponArtMenuWaitingTitle", "Waiting for weapon art data")},
        {"waitingBody",
         ResolveLabel("WeaponArtMenuWaitingBody",
                      "Once the plugin sends the catalog, the selected entry will appear here.")},
        {"unlockLevel", ResolveLabel("WeaponArtMenuUnlockLevel", "Unlock Level")},
        {"pointCost", ResolveLabel("WeaponArtMenuPointCost", "Point Cost")},
        {"activation", ResolveLabel("WeaponArtMenuActivation", "Activation")},
        {"assignment", ResolveLabel("WeaponArtMenuAssignment", "Assignment")},
        {"available", ResolveLabel("WeaponArtMenuAvailable", "Available")},
        {"currentlyAssigned", ResolveLabel("WeaponArtMenuCurrentlyAssigned", "Currently Assigned")},
        {"unlocked", ResolveLabel("WeaponArtMenuUnlocked", "Unlocked")},
        {"locked", ResolveLabel("WeaponArtMenuLocked", "Locked")},
        {"levelBadge", ResolveLabel("WeaponArtMenuLevelBadge", "Lv {}")},
        {"costBadge", ResolveLabel("WeaponArtMenuCostBadge", "Cost {}")},
        {"prepare", ResolveLabel("WeaponArtMenuPrepare", "Prepared")},
        {"instant", ResolveLabel("WeaponArtMenuInstant", "Instant")},
        {"compatible", ResolveLabel("WeaponArtMenuCompatible", "Compatible")},
        {"incompatible", ResolveLabel("WeaponArtMenuIncompatible", "Not compatible")},
        {"assigned", ResolveLabel("WeaponArtMenuAssigned", "Assigned")},
        {"alreadyUnlocked", ResolveLabel("WeaponArtMenuAlreadyUnlocked", "Already Unlocked")},
        {"unlockAction", ResolveLabel("WeaponArtMenuUnlockAction", "Unlock Art ({} pt)")},
        {"assignedToWeapon", ResolveLabel("WeaponArtMenuAssignedToWeapon", "Assigned to Weapon")},
        {"assignToSelectedWeapon",
         ResolveLabel("WeaponArtMenuAssignToSelectedWeapon", "Assign to Selected Weapon")},
        {"hintSelectArt",
         ResolveLabel("WeaponArtMenuHintSelectArt",
                      "Select a weapon art from the grid to bind it to this weapon.")},
        {"hintCanBind", ResolveLabel("WeaponArtMenuHintCanBind",
                                     "Bind {} to this weapon, or clear the current binding.")},
        {"hintHasBinding",
         ResolveLabel("WeaponArtMenuHintHasBinding",
                      "This weapon already has a bound weapon art. You can replace it with the "
                      "highlighted compatible art or clear it.")},
        {"hintNoBinding",
         ResolveLabel("WeaponArtMenuHintNoBinding", "Select an unlocked compatible weapon art to "
                                                    "bind it, or leave this weapon unassigned.")},
        {"hintUnlockBlocked", ResolveLabel("WeaponArtMenuHintUnlockBlocked",
                                           "Requires level {} and {} available point(s).")},
        {"hintNeedWeapon",
         ResolveLabel("WeaponArtMenuHintNeedWeapon",
                      "Select a weapon in the inventory to enable weapon-specific assignment.")},
        {"hintIncompatible",
         ResolveLabel("WeaponArtMenuHintIncompatible",
                      "The selected weapon does not meet this art's weapon requirements.")},
        {"hintAlreadyAssigned", ResolveLabel("WeaponArtMenuHintAlreadyAssigned",
                                             "This weapon is already using the highlighted art.")},
        {"hintReadyToAssign",
         ResolveLabel("WeaponArtMenuHintReadyToAssign",
                      "This art is ready to be assigned to the currently selected weapon.")},
        {"hintUnlockFirst",
         ResolveLabel("WeaponArtMenuHintUnlockFirst",
                      "Unlock this art first, then it can be assigned to the selected weapon.")},
        {"hintStateUpdated",
         ResolveLabel("WeaponArtMenuHintStateUpdated", "Weapon art state updated.")}};
  }

  json BuildWeaponArtHUDStrings()
  {
    return json{{"label", ResolveLabel("WeaponArtHUDLabel", "Weapon Art")},
                {"defaultName", ResolveLabel("WeaponArtUnassigned", "Unassigned")},
                {"disabled", ResolveLabel("WeaponArtHUDStateDisable", "Disabled")},
                {"preparing", ResolveLabel("WeaponArtHUDStatePrepare", "Preparing")},
                {"enabled", ResolveLabel("WeaponArtHUDStateEnable", "Enabled")}};
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
                                  ResolveLabel("UnknownWeaponType", "Unknown"))},
        {"currentArtId", currentArtID},
        {"currentArtName", ResolveWeaponArtName(currentArtID)}};
  } else {
    payload["selectedWeapon"] = nullptr;
  }

  auto arts = json::array();
  for (const auto* art : WeaponArt::Manager::GetAllWeaponArts()) {
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
  if (!prisma || !prisma->IsValid(view) || !Settings::bUseWeaponArtHUD)
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
                  {"label", ResolveLabel("WeaponArtHUDLabel", "Weapon Art")},
                  {"defaultName", ResolveLabel("WeaponArtUnassigned", "Unassigned")}};

  auto data = payload.dump();
  prisma->InteropCall(view, "setHudConfig", data.c_str());
}
}  // namespace UI