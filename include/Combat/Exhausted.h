#pragma once

class Exhausted
{
public:
  static Exhausted& GetSingleton()
  {
    static Exhausted singleton;
    return singleton;
  }

  static bool IsActorExhausted(RE::Actor* actor);
  static void EnterExhausted(RE::Actor* actor);
  static void ExitExhausted(RE::Actor* actor);

private:
  Exhausted();
  static inline std::mutex mtx;

  // 力竭Actor数据序列化ID
  constexpr static inline std::uint32_t exhausted = 'REXH';
  static inline std::unordered_set<RE::FormID> exhaustedActors;
  static inline std::unordered_set<RE::Actor*> runtimeExhaustedActors;
};