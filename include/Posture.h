#pragma once

#include "Settings.h"

#include "API/TrueHUDAPI.h"

class Posture
{
public:
  ~Posture() { ReleaseHUD(); }

  static Posture& GetSingleton()
  {
    static Posture singleton;
    return singleton;
  }

  static float InitPosture(RE::Actor* actor);
  static float GetCurrentPosture(RE::Actor* actor);
  static float GetMaxPosture(RE::Actor* actor);

  void ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData);
  void ModPostureValue(RE::Actor* actor, float value);

  void InitHUD();
  void ReleaseHUD();

  static bool IsActorExhausted(RE::Actor* a_actor);
  void EnterExhausted(RE::Actor* a_actor);
  void QuitExhausted(RE::Actor* a_actor);

private:
  constexpr static std::string_view CurrentPostureHealthName = "fRimPostureHealth";
  constexpr static std::string_view MaxPostureHealthName     = "fRimPostureMax";

  static std::mutex postureMutex;

  TRUEHUD_API::IVTrueHUD4* hud{nullptr};
};
