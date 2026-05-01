#pragma once

namespace UI
{
using PrismaView = std::uint64_t;

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

  void EnterGeryOut(RE::Actor* actor);
  void ExitGreyOut(RE::Actor* actor);

private:
  TrueHUD();
  ~TrueHUD();

  bool init{false};
};

class WeaponArtInfoCart
{
public:
  static WeaponArtInfoCart& GetSingleton()
  {
    static WeaponArtInfoCart singleton;
    return singleton;
  }

private:
  PrismaView view{0};
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