#include "GUI/UI.h"

#include "Combat/Posture.h"
#include "Combat/Weapon.h"
#include "Combat/WeaponArt.h"
#include "Core/Settings.h"
#include "Utils.h"

#include "API/PrismaUI_API.h"
#include "API/TrueHUDAPI.h"

#include <cstdlib>

#include "nlohmann/json.hpp"

namespace UI
{
static PRISMA_UI_API::IVPrismaUI2* prisma = nullptr;
static TRUEHUD_API::IVTrueHUD4* truehud   = nullptr;

namespace
{
  using json = nlohmann::json;

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

  std::string DamageTypeToString(WeaponArt::WeaponArtInfo::DamageType type)
  {
    using DamageType = WeaponArt::WeaponArtInfo::DamageType;

    switch (type) {
    case DamageType::Slash:
      return "Slash";
    case DamageType::Thrust:
      return "Thrust";
    case DamageType::Strike:
      return "Strike";
    case DamageType::Magic:
      return "Magic";
    case DamageType::Fire:
      return "Fire";
    case DamageType::Frost:
      return "Frost";
    case DamageType::Shock:
      return "Shock";
    case DamageType::None:
    default:
      return "None";
    }
  }

  std::string WeaponTypeToString(Weapon::Type type)
  {
    switch (type) {
    case Weapon::Type::Unarm:
      return "Unarmed";
    case Weapon::Type::Werewolf:
      return "Werewolf";
    case Weapon::Type::VampireLord:
      return "Vampire Lord";
    case Weapon::Type::Dagger:
      return "Dagger";
    case Weapon::Type::Sword:
      return "Sword";
    case Weapon::Type::Axe:
      return "Axe";
    case Weapon::Type::Mace:
      return "Mace";
    case Weapon::Type::GreatSword:
      return "Greatsword";
    case Weapon::Type::GreatAxe:
      return "Greataxe";
    case Weapon::Type::GreatMace:
      return "Greatmace";
    case Weapon::Type::Bow:
      return "Bow";
    case Weapon::Type::Crossbow:
      return "Crossbow";
    case Weapon::Type::Staff:
      return "Staff";
    case Weapon::Type::Shield:
      return "Shield";
    case Weapon::Type::Fist:
      return "Fist";
    case Weapon::Type::Claw:
      return "Claw";
    case Weapon::Type::Rapier:
      return "Rapier";
    case Weapon::Type::Katana:
      return "Katana";
    case Weapon::Type::ShortSpear:
      return "Short Spear";
    case Weapon::Type::Halberd:
      return "Halberd";
    case Weapon::Type::Spear:
      return "Spear";
    case Weapon::Type::Quarterstaff:
      return "Quarterstaff";
    case Weapon::Type::GreatKatana:
      return "Great Katana";
    default:
      return "Unknown";
    }
  }

  void PrismaConsoleCallback(PrismaView view, PRISMA_UI_API::ConsoleMessageLevel level,
                             const char* message)
  {
    auto msg = message ? message : "";

    switch (level) {
    case PRISMA_UI_API::ConsoleMessageLevel::Error:
      logger::error("Prisma[{}]: {}", view, msg);
      break;
    case PRISMA_UI_API::ConsoleMessageLevel::Warning:
      logger::warn("Prisma[{}]: {}", view, msg);
      break;
    case PRISMA_UI_API::ConsoleMessageLevel::Debug:
    case PRISMA_UI_API::ConsoleMessageLevel::Info:
    case PRISMA_UI_API::ConsoleMessageLevel::Log:
    default:
      logger::info("Prisma[{}]: {}", view, msg);
      break;
    }
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

  prisma->SetOrder(view, 120);
  prisma->Hide(view);
  prisma->RegisterJSListener(view, "closeWeaponArtMenu", Close);
  prisma->RegisterJSListener(view, "unlockWeaponArt", UnlockWeaponArt);
  prisma->RegisterJSListener(view, "setWeaponArt", SetWeaponArt);
  prisma->RegisterConsoleCallback(view, PrismaConsoleCallback);
}

WeaponArtMenu::~WeaponArtMenu()
{
  if (prisma && prisma->IsValid(view)) {
    prisma->Unfocus(view);
    prisma->Destroy(view);
  }

  view = 0;
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
  logger::info("UI: WeaponArtMenu DOM ready: {}", readyView);

  if (!isOpen && prisma && prisma->IsValid(readyView))
    prisma->Hide(readyView);

  if (isOpen)
    SyncViewData();
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
    currentArtID         = WeaponArt::Manager::GetWeaponArtID(selectedWeapon);
    auto* currentArtInfo = WeaponArt::Manager::GetWeaponArtInfo(currentArtID);
    auto* weaponName     = selectedWeapon->GetName();

    payload["selectedWeapon"] = {
        {"name", weaponName ? weaponName : ""},
        {"formId", std::format("{:08X}", selectedWeapon->GetFormID())},
        {"type", WeaponTypeToString(Weapon::GetWeaponType(selectedWeapon))},
        {"currentArtId", currentArtID},
        {"currentArtName", currentArtInfo ? currentArtInfo->GetName() : "Unassigned"}};
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
                    {"damageType", DamageTypeToString(art->GetDamageType())},
                    {"damageMult", art->GetDamageMult()},
                    {"baseDamage", art->GetBaseDamage()},
                    {"postureDamageMult", art->GetPostureDamageMult()},
                    {"unlocked", unlocked},
                    {"weaponAllowed", weaponAllowed},
                    {"isAssigned", selectedWeapon && currentArtID == art->GetID()}});
  }

  payload["arts"] = std::move(arts);

  auto data = payload.dump();
  prisma->InteropCall(view, "setMenuState", data.c_str());
}

void WeaponArtMenu::Toggle()
{
  if (isOpen)
    Hide();
  else
    Show();
}

void WeaponArtMenu::Show()
{
  if (!prisma || !prisma->IsValid(view) || !inventoryMenuOpen)
    return;

  isOpen = true;
  prisma->Show(view);
  if (domReady)
    SyncViewData();

  if (!prisma->HasFocus(view) && !prisma->Focus(view, false, true))
    logger::warn("UI: WeaponArtMenu focus failed.");
}

void WeaponArtMenu::Hide()
{
  if (!prisma || !prisma->IsValid(view))
    return;

  isOpen = false;
  if (prisma->HasFocus(view))
    prisma->Unfocus(view);
  prisma->Hide(view);
}

void WeaponArtMenu::Close(const char*)
{
  Hide();
}

void WeaponArtMenu::SetInventoryMenuOpen(bool open)
{
  inventoryMenuOpen = open;
  if (!open && isOpen)
    Hide();
}

void WeaponArtMenu::UnlockWeaponArt(const char* arg)
{
  auto id = ParseArtID(arg);
  if (!id)
    return;

  auto* art = WeaponArt::Manager::GetWeaponArtInfo(*id);
  if (!art)
    return;

  auto alreadyUnlocked = WeaponArt::PlayerStat::IsUnlocked(*id);
  auto unlocked        = WeaponArt::PlayerStat::UnlockArt(*art);
  if (unlocked && !alreadyUnlocked)
    logger::info("WeaponArtMenu: unlocked {} ({})", art->GetName(), *id);
  else if (!unlocked)
    logger::info("WeaponArtMenu: unlock failed for {} ({})", art->GetName(), *id);

  SyncViewData();
}

void WeaponArtMenu::SetWeaponArt(const char* arg)
{
  auto id = ParseArtID(arg);
  if (!id)
    return;

  auto* art = WeaponArt::Manager::GetWeaponArtInfo(*id);
  if (!art || !WeaponArt::PlayerStat::IsUnlocked(*id))
    return;

  auto* weapon = GetSelectedWeapon();
  if (!weapon || !art->IsWeaponAllowed(weapon))
    return;

  WeaponArt::Manager::SetWeaponArtInfo(weapon, *id);

  if (auto* player = RE::PlayerCharacter::GetSingleton())
    WeaponArt::Manager::EnableWeaponArt(player, WeaponArt::Manager::IsEnabled(player));

  logger::info("WeaponArtMenu: set {} ({}) on weapon {:08X}", art->GetName(), *id,
               weapon->GetFormID());
  SyncViewData();
}

}  // namespace UI