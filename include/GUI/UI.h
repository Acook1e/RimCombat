#pragma once

namespace UI
{
using PrismaView = std::uint64_t;

void Initialize();

class TrueHUD
{
public:
  static TrueHUD& GetSingleton()
  {
    static TrueHUD singleton;
    return singleton;
  }

  static bool Require();
  static bool Release();

  static void EnterGreyOut(RE::Actor* actor);
  static void ExitGreyOut(RE::Actor* actor);

private:
  TrueHUD();
  ~TrueHUD();

  static inline bool init{false};
};

class WeaponArtMenu
{
public:
  static WeaponArtMenu& GetSingleton()
  {
    static WeaponArtMenu singleton;
    return singleton;
  }

  static bool IsShow() { return isShow; }
  static bool IsInventoryMenuShow() { return inventoryMenuShow; }

  static void Toggle();
  static void Show();
  static void Hide();
  static void SetInventoryMenuOpen(bool open);
  static void UnlockWeaponArt(const char* arg);
  static void SetWeaponArt(const char* arg);

private:
  WeaponArtMenu();
  ~WeaponArtMenu();

  static void OnViewReady(PrismaView readyView);
  static void SyncViewData();
  static RE::TESObjectWEAP* GetSelectedWeapon();

  static inline PrismaView view{0};
  static inline bool isShow{false};
  static inline bool domReady{false};
  static inline bool inventoryMenuShow{false};
};

class WeaponArtHUD
{
public:
  static WeaponArtHUD& GetSingleton()
  {
    static WeaponArtHUD singleton;
    return singleton;
  }

  static bool IsShow() { return isShow; }
  static void Show();
  static void Hide();

  static void UpdateState(bool enable);
  static void UpdateName(std::int32_t artID);

private:
  WeaponArtHUD();
  ~WeaponArtHUD();

  static void OnViewReady(PrismaView readyView);
  static void SyncViewConfig();
  static std::string ResolveWeaponArtName(std::int32_t artID);

  static inline PrismaView view{0};
  static inline bool isShow{false};
  static inline bool domReady{false};
};
}  // namespace UI