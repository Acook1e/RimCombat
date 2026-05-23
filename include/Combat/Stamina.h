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

  // 对于使用原版swing事件的攻击，根据swing类型消耗耐力
  static void SwingStaminaConsume(RE::Actor* actor, bool leftAttack, bool unarm = false);

  static void PrecisionStart(RE::Actor* actor);
  static void PrecisionEnd(RE::Actor* actor);
  // 对于使用Precion系统的攻击，根据添加的节点判断左右手攻击并消耗耐力
  static void CollisionStaminaConsume(RE::Actor* actor, const std::string& payload);

  // RimCombat的耐力系统具有最高的优先级，Precision系统和原版swing事件都不处理使用RimCombat耐力系统的角色的攻击耐力消耗

  static void Start(RE::Actor* actor);
  static void End(RE::Actor* actor);
  static void Consume(RE::Actor* actor, const std::string& payload);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Stamina();
  // Rim Combat Stamina System
  constexpr static std::uint32_t serialType = 'RCSS';

  // 缓存使用Precision系统的角色
  static inline std::mutex mtx_precision;
  static inline std::unordered_set<RE::Actor*> usePrecisionStaminaActors;

  // 缓存使用RimCombat耐力系统的角色
  static inline std::mutex mtx_rimStamina;
  static inline std::unordered_set<RE::Actor*> useRimStaminaActors;
};