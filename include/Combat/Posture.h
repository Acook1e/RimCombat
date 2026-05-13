#pragma once

class Posture
{
public:
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

  static void ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData,
                              bool isTimedBlock);
  static void DamagePostureValue(RE::Actor* actor, float value, bool ignoreBreak = false);

private:
  Posture();
  static float CalculateMaxPosture(RE::Actor* actor);
  static float InitPosture(RE::Actor* actor);
  static PostureData& GetPostureDataRef(RE::Actor* actor);

  // 架势数据序列化ID
  // Rim Combat Posture Data
  constexpr static inline std::uint32_t serialType = 'RCPD';

  static inline std::mutex mtx;
  static inline std::unordered_map<RE::FormID, PostureData> postureMap;
  static inline std::unordered_map<RE::Actor*, PostureData> runtimePostureMap;
};
