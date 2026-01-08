#pragma once

#include "pch.h"

namespace Handler::Poise
{
class RimPoise
{
public:
  static RimPoise& GetSingleton()
  {
    static RimPoise singleton;
    return singleton;
  }

  void ProcessMeleeHit(RE::Actor* attacker, RE::Actor* victim, RE::HitData& hitData);

  float CalculateMaxPoise(RE::Actor* actor);
  float GetCurrentPoise(RE::Actor* actor);
  float GetMaxPoise(RE::Actor* actor);

  void RestorePoiseHealth(RE::Actor* actor, float a_percent);
  void DamagePoiseHealth(RE::Actor* actor, float poiseDamage);
  void ResetPoiseHealth(RE::Actor* actor);

private:
  static constexpr std::string_view CurrentPoiseStr = "fRimPoiseHealth";
  static constexpr std::string_view MaxPoiseStr     = "fRimPoiseMax";

  std::mutex mtx;
};

inline float GetCurrentPoise(RE::Actor* actor)
{
  return RimPoise::GetSingleton().GetCurrentPoise(actor);
}
inline float GetMaxPoise(RE::Actor* actor)
{
  return RimPoise::GetSingleton().GetMaxPoise(actor);
}
}  // namespace Handler::Poise