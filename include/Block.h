#pragma once

class Block
{
public:
  static Block& GetSingleton()
  {
    static Block singleton;
    return singleton;
  }
  void StartBlock(RE::Actor* a_actor);
  void EndBlock(RE::Actor* a_actor);

  bool IsTimedBlock(RE::Actor* a_actor);

private:
  int64_t lastCleanTime = 0;
  std::unordered_map<RE::Actor*, std::int64_t> blockStartTimes;
  std::mutex mutex;
};