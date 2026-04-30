#pragma once

#include "Core/Settings.h"

class Posture
{
public:
  struct PostureData
  {
    float current;
    float max;
  };

  static Posture& GetSingleton()
  {
    static Posture singleton;
    return singleton;
  }

  static float InitPosture(RE::Actor* actor);
  static void ReCalculateMaxPosture(RE::Actor* actor);

  static float GetCurrentPosture(RE::Actor* actor);
  static float GetMaxPosture(RE::Actor* actor);
  static PostureData& GetPostureData(RE::Actor* actor);

  void ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim,
                       RE::HitData& hitData, bool isTimedBlock);
  void ModPostureValue(RE::Actor* actor, float value, bool ignoreBreak = false);
  void PostureBreak(RE::Actor* actor);

private:
  Posture();
  static inline std::mutex mtx;

  // 架势数据序列化ID
  constexpr static inline std::uint32_t posture = 'RPOS';
  static inline std::unordered_map<RE::FormID, PostureData> postureMap;
  static inline std::unordered_map<RE::Actor*, PostureData> runtimePostureMap;
};
