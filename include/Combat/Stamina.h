#pragma once

#include "Core/Settings.h"

class Stamina
{
public:
  static Stamina& GetSingleton()
  {
    static Stamina singleton;
    return singleton;
  }

  static void AttackStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm = false);
  static void WeaponArtStaminaConsume(RE::Actor* actor, std::string payload);

private:
  Stamina() = default;
};