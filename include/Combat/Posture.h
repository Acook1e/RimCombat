#pragma once

class Posture
{
public:
  // bool型变量
  // 表示是否可以架势崩溃
  constexpr static std::string_view BREAKABLE = "RimCombat_PostureBreakable";

  // 图事件，payload用于传递信息
  // Breakable|flag用于设置当前是否可以架势崩溃，flag为bool值，true表示可以崩溃，false表示不可崩溃
  // Damage|multiplier|fallbackMultiplier用于设置架势伤害倍率
  // multiplier和fallbackMultiplier是float值，不处理小于0的值
  // 当满足战技条件时使用multiplier的倍率，不满足战技条件时使用fallbackMultiplier的倍率
  // 在非战技中使用multiplier的倍率，可以不写fallbackMultiplier，因为根本不会处理
  // End用于结束当前攻击的架势处理，清空缓存的Damage|multiplier事件，通常在此次攻击的命中帧结束时触发
  constexpr static std::string_view RIMPOSTURE = "RimPosture";

  struct PostureData
  {
    float current = 0.0f;
    float max     = 0.0f;

    // 上次受到架势伤害的时间，用于计算架势恢复延迟
    std::uint64_t lastDamageTime = 0;
  };

  static Posture& GetSingleton()
  {
    static Posture singleton;
    return singleton;
  }

  static void Update(std::uint64_t deltaTime = 0);

  static void ReCalculateMaxPosture(RE::Actor* actor);

  static float GetCurrentPosture(RE::Actor* actor);
  static float GetMaxPosture(RE::Actor* actor);
  static PostureData GetPostureData(RE::Actor* actor);

  static void ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData);
  static void DamagePostureValue(RE::Actor* actor, float value, bool ignoreBreak = false);

  static void Damage(RE::Actor* actor, const std::string& payload);
  static void End(RE::Actor* actor);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Posture();
  static float CalculateMaxPosture(RE::Actor* actor);
  static float InitPosture(RE::Actor* actor);
  static PostureData& GetPostureDataRef(RE::Actor* actor);

  // 架势数据序列化ID
  // Rim Combat Posture Data
  constexpr static inline std::uint32_t serialType = 'RCPD';

  static inline std::mutex mtx_postureData;
  static inline std::unordered_map<RE::FormID, PostureData> postureMap;
  static inline std::unordered_map<RE::Actor*, PostureData> runtimePostureMap;

  // 缓存每次攻击的架势伤害，直到攻击结束时再处理
  static inline std::mutex mtx_damageCache;
  static inline std::unordered_map<RE::Actor*, float> damageCache;
};
