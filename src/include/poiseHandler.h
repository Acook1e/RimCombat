#pragma once

#include "pch.h"

namespace Handler::PoiseHandler
{

static constexpr std::string_view CurrentMaxsuPoise_Name   = "MaxsuPoise_PoiseHealth";
static constexpr std::string_view CurrentPoiseHandler_Name = "RimCombatPoise_PoiseHealth";

static constexpr std::string_view MaxPoiseHandler_Name = "RimCombatPoise_PoiseMax";

class RimCombatPoise
{
public:
  static RimCombatPoise& GetSingleton()
  {
    static RimCombatPoise singleton;
    return singleton;
  }

  void ProcessHit(RE::Actor* attacker, RE::Actor* victim, RE::HitData& hitData);

  float CalculateMaxPoise(RE::Actor* actor);
  float GetCurrentPoise(RE::Actor* actor);
  float GetMaxPoise(RE::Actor* actor);

  void RestorePoiseHealth(RE::Actor* actor, float a_percent);
  void DamagePoiseHealth(RE::Actor* actor, float poiseDamage);
  void ResetPoiseHealth(RE::Actor* actor);

private:
  std::mutex mtx;
};

float GetCurrentPoise(RE::Actor* actor);
float GetMaxPoise(RE::Actor* actor);
}  // namespace Handler::PoiseHandler