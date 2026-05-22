#pragma once

class Posture
{
public:
  // 图事件，payload用于传递信息
  // Unbreakable|Duration用于设置当前免疫架势崩溃，Duration为持续时间，单位为毫秒
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

    // 架势恢复延迟结束的时间点，单位为毫秒
    std::uint64_t regenResumeTime = 0;
  };

  static Posture& GetSingleton()
  {
    static Posture singleton;
    return singleton;
  }

  static void Update(std::uint64_t deltaTime = 0);

  // 最大架势的计算仅和最大生命值及最大耐力相关
  static void UpdateMaxPosture(RE::Actor* actor);

  static float GetCurrentPosture(RE::Actor* actor);
  static float GetMaxPosture(RE::Actor* actor);
  static PostureData GetPostureData(RE::Actor* actor);

  static void ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData);
  static void DamagePostureHealth(RE::Actor* actor, float value, bool ignoreBreak = false);

  static void Unbreakable(RE::Actor* actor, const std::string& payload);
  static void Damage(RE::Actor* actor, const std::string& payload);
  static void End(RE::Actor* actor);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Posture();
  static float InitPosture(RE::Actor* actor);
  static float CalculateMaxPosture(RE::Actor* actor);
  static PostureData& GetPostureDataRef(RE::Actor* actor);

  // 架势数据序列化ID
  // Rim Combat Posture Data
  constexpr static inline std::uint32_t serialType = 'RCPD';

  // 读取频繁，使用共享锁和序列化
  static inline std::shared_mutex mtx_postureData;
  static inline std::unordered_map<RE::FormID, PostureData> postureMap;
  static inline std::unordered_map<RE::Actor*, PostureData> runtimePostureMap;

  // 缓存每次攻击的架势伤害，直到攻击结束时再处理
  static inline std::mutex mtx_damageCache;
  static inline std::unordered_map<RE::Actor*, float> damageCache;

  // 缓存不可破防状态的持续时间
  static inline std::mutex mtx_unbreakableCache;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> unbreakableActors;
};
