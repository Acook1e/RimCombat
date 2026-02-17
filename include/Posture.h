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
  static void ReCalculateMaxPosture(RE::Actor* actor);

  static float GetCurrentPosture(RE::Actor* actor);
  static float GetMaxPosture(RE::Actor* actor);

  void ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData, bool isTimedBlock);
  void ModPostureValue(RE::Actor* actor, float value, bool ignoreBreak = false);
  void PostureBreak(RE::Actor* actor);

  void InitHUD();
  void ReleaseHUD();

  static bool IsActorExhausted(RE::Actor* a_actor);
  void EnterExhausted(RE::Actor* a_actor);
  void ExitExhausted(RE::Actor* a_actor);

private:
  constexpr static std::string_view CurrentPostureHealthName = "fRimPostureHealth";
  constexpr static std::string_view MaxPostureHealthName     = "fRimPostureMax";

  static std::mutex postureMutex;

  TRUEHUD_API::IVTrueHUD4* hud{nullptr};
};
