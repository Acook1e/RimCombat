#pragma once

#include "Data/Race.h"

class Stamina
{
public:
  // 图事件，payload用于传递信息
  // Start表示这次攻击使用RimCombat的耐力系统，weaponSwing通道的耐力消耗不会生效
  // End表示此动画的耐力消耗结束，恢复原版的耐力系统
  // Consume|AttackType|Side|Multiplier| 表示基于某个攻击类型的耐力消耗
  // AttackType可以是Normal或Power
  // Side为可以是Left或Right或Creature以及Auto
  // Multiplier是耐力消耗倍率，不处理小于0的值
  constexpr static std::string_view RIMSTAMINA = "RimStamina";

  static Stamina& GetSingleton()
  {
    static Stamina singleton;
    return singleton;
  }

  enum Side : std::uint8_t
  {
    None,
    Left,
    Right,
    Creature,
    Auto
  };

  // 由AttackState的切换触发
  static void SwingStaminaConsume(RE::Actor* actor, RE::TESObjectWEAP* weapon);
  static void CreatureStaminaConsume(RE::Actor* actor, Race::Type raceType);
  static void BashStaminaConsume(RE::Actor* actor);

  // AttackState无法正确获取空手攻击，使用图事件触发
  static void UnarmStaminaConsume(RE::Actor* actor);

  // RimCombat的耐力系统具有最高的优先级
  static void RimStaminaConsume(RE::Actor* actor, Side side, bool power, float multiplier);

  static void Start(RE::Actor* actor);
  static void End(RE::Actor* actor);
  static void Consume(RE::Actor* actor, const std::string& payload);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Stamina();
  // Rim Combat Stamina System
  constexpr static std::uint32_t serialType = 'RCSS';

  // 缓存使用RimCombat耐力系统的角色
  static inline std::mutex mtx_rimStamina;
  static inline std::unordered_set<RE::Actor*> useRimStaminaActors;
};