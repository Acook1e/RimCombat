#pragma once

class Block
{
public:
  static Block& GetSingleton()
  {
    static Block singleton;
    return singleton;
  }
  void StartBlock(RE::Actor* actor);
  void EndBlock(RE::Actor* actor);

  bool IsTimedBlock(RE::Actor* actor);

private:
  int64_t lastCleanTime = 0;
  static inline std::unordered_map<RE::Actor*, std::int64_t> blockStartTimes;
  static inline std::mutex mutex;
};