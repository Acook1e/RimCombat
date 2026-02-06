#pragma once

#include "Settings.h"

class Stamina
{
public:
  static Stamina& GetSingleton()
  {
    static Stamina singleton;
    return singleton;
  }
  static float AttackStaminaConsume(RE::Actor* a_actor, bool powerAttack, bool consume, bool unarmed = false);
};