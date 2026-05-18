#pragma once

#include "Core/Settings.h"

class Stamina
{
public:
  // 图事件，payload用于传递信息
  // Attack|Side|xxx 表示基于轻攻击类型的耐力消耗
  // PowerAttack|Side|xxx 表示基于重攻击类型的耐力消耗
  // Side为可以是Left或Right以及Auto，xxx是耐力消耗倍率，不处理小于0的值
  constexpr static std::string_view RIMSTAMINA = "RimStamina";

  static Stamina& GetSingleton()
  {
    static Stamina singleton;
    return singleton;
  }

  static void AttackStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm = false);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Stamina() = default;
};