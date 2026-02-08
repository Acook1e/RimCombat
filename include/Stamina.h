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
  static void AttackStaminaConsume(RE::Actor* a_actor, bool leftAttack);
};