#pragma once

class Block
{
public:
  static Block& GetSingleton()
  {
    static Block singleton;
    return singleton;
  }
  // 挂在主线程更新中，定期清理过期的格挡时间记录
  static void Update();

  static void StartBlock(RE::Actor* actor);
  static void EndBlock(RE::Actor* actor);

  // 在一次攻击中判断，否则无效
  // 成功会发发送一个限时格挡事件
  static bool IsTimedBlock(RE::Actor* actor);

  static void AddTimeBlockListener(std::function<void(RE::Actor*)> callback);

private:
  Block();
  // Rim Combat Block Revert
  // 用于读档的时候清空所有记录
  constexpr static inline std::uint32_t serialType = 'RCBR';

  static inline std::mutex mtx;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> blockStartTimes;
  static inline std::vector<std::function<void(RE::Actor*)>> timeBlockCallbacks;
};