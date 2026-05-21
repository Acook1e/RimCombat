#pragma once

#include "Core/Settings.h"

class Stamina
{
public:
  // 图事件，payload用于传递信息
  // Start表示这次攻击使用RimCombat的耐力系统，weaponSwing通道的耐力消耗不会生效
  // End表示此动画的耐力消耗结束，恢复原版的耐力系统
  // Consume|AttackType|Side|Multiplier|FallbackMultiplier 表示基于某个攻击类型的耐力消耗
  // AttackType可以是Normal或Power
  // Side为可以是Left或Right以及Auto
  // Multiplier和FallbackMultiplier是耐力消耗倍率，不处理小于0的值
  // Multiplier代表在满足战技条件下的倍率，FallbackMultiplier代表不满足战技条件下的倍率
  // 非战技中使用Multiplier的倍率，可以不写FallbackMultiplier，因为根本不会处理
  constexpr static std::string_view RIMSTAMINA = "RimStamina";

  static Stamina& GetSingleton()
  {
    static Stamina singleton;
    return singleton;
  }

  static void SwingStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm = false);

  static void Start(RE::Actor* actor);
  static void End(RE::Actor* actor);
  static void Consume(RE::Actor* actor, const std::string& payload);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Stamina();
  // Rim Combat Stamina System
  constexpr static std::uint32_t serialType = 'RCSS';

  // 缓存使用RimCombat耐力系统的角色
  static inline std::mutex mtx;
  static inline std::unordered_set<RE::Actor*> useRimStaminaActors;
};