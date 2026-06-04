#pragma once

#include "Combat/Stagger.h"
#include "Core/Settings.h"

class Poise
{
public:
  // 图事件，payload用于传递信息
  // Set|multiplier表示自身受到韧性伤害，multiplier为韧性伤害倍率
  // End表示结束对自身的韧性伤害修改，清空缓存的Set|multiplier事件，通常在此次攻击的命中帧结束时触发
  // TargetSet|multiplier|fallback用于设置击中目标的韧性伤害倍率，multiplier为韧性伤害倍率
  // fallback用于战技条件不满足时的韧性伤害倍率，multiplier和fallback都不处理小于0的值
  // TargetEnd用于结束对目标的韧性伤害倍率设置，通常在此次攻击命中帧结束时触发
  constexpr static std::string_view RIMPOISE = "RimPoise";

  struct PoiseData
  {
    // 允许当前超过上限
    float current = 0.0f;
    float max     = 0.0f;

    std::uint64_t regenResumeTime = 0;
  };

  static Poise& GetSingleton()
  {
    static Poise singleton;
    return singleton;
  }

  static void Update(std::uint64_t deltaTime = 0);

  // 最大韧性的计算仅和最大耐力和护甲类型相关
  static void UpdateMaxPoise(RE::Actor* actor);

  static Stagger::Level CalculateStaggerLevel(float poiseDamage);

  // 韧性不需要HUD显示，用于外部API查询和内部处理
  static PoiseData GetPoiseData(RE::Actor* actor);

  static void ProcessWeaponHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData);
  static void DamagePoiseHealth(RE::Actor* actor, float value);

  static void Set(RE::Actor* actor, const std::string& payload);
  static void End(RE::Actor* actor);
  static void TargetSet(RE::Actor* actor, const std::string& payload);
  static void TargetEnd(RE::Actor* actor);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Poise();
  static void InitPoiseData(RE::Actor* actor);
  static float CalculateMaxPoise(RE::Actor* actor);
  static PoiseData& GetPoiseDataRef(RE::Actor* actor);

  // Rim Combat Poise System
  constexpr static inline std::uint32_t serialType = 'RCPS';

  // 无锁，仅初始化时写入，之后只读取
  // 来源于设置的对种族或者Actor的基础韧性覆盖
  static inline std::unordered_map<RE::FormID, float> racePoiseOverrideMap;
  static inline std::unordered_map<RE::FormID, float> actorPoiseOverrideMap;

  // 需要锁和序列化
  // Actor的韧性数据
  static inline std::mutex mtx_poiseData;
  static inline std::unordered_map<RE::Actor*, PoiseData> poiseDataMap;

  // 需要锁
  // Actor的韧性伤害倍率缓存
  // OnHit是被存入的Actor作为攻击者时应用
  // Self是被存入的Actor作为受击者时应用
  static inline std::mutex mtx_poiseMultiplier;
  static inline std::unordered_map<RE::Actor*, float> poiseMultOnHit;
  static inline std::unordered_map<RE::Actor*, float> poiseMultSelf;
};