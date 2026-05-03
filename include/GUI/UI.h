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

  bool Require();
  bool Release();

  void EnterGreyOut(RE::Actor* actor);
  void ExitGreyOut(RE::Actor* actor);

private:
  TrueHUD();
  ~TrueHUD();

  bool init{false};
};

class WeaponArtMenu
{
public:
  static WeaponArtMenu& GetSingleton()
  {
    static WeaponArtMenu singleton;
    return singleton;
  }

  static bool IsOpen() { return isOpen; }
  static bool IsInventoryMenuOpen() { return inventoryMenuOpen; }

  static void Toggle();
  static void Show();
  static void Hide();
  static void Close(const char* arg);
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
  static inline bool isOpen{false};
  static inline bool domReady{false};
  static inline bool inventoryMenuOpen{false};
};

class WeaponArtHUD
{
public:
  static WeaponArtHUD& GetSingleton()
  {
    static WeaponArtHUD singleton;
    return singleton;
  }

private:
  PrismaView view{0};
};
}  // namespace UI