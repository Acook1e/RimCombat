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

  static void Show();
  static void Hide();

  static void SetInventoryMenuOpen(bool open)
  {
    inventoryMenuOpen = open;
    if (!open && isOpen)
      Hide();
  }

  static void UnlockWeaponArt(const char* arg)
  {
    if (!arg || !*arg)
      return;

    char* end       = nullptr;
    std::int32_t id = std::strtol(arg, &end, 10);
    if (end == arg || *end != '\0')
      return;

    logger::info("Unlocking Weapon Art with ID: {}", id);
  }

  static void SetWeaponArt(const char* arg)
  {
    if (!arg || !*arg)
      return;

    char* end       = nullptr;
    std::int32_t id = std::strtol(arg, &end, 10);
    if (end == arg || *end != '\0')
      return;

    logger::info("Setting Weapon Art Info with ID: {}", id);
  }

private:
  WeaponArtMenu();
  ~WeaponArtMenu();

  static inline PrismaView view{0};
  static inline bool isOpen{false};
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