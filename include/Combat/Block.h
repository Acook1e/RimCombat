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

  static void ProcessBlock(RE::Actor* actor);
  static void ProcessDamage(RE::Actor* victim, RE::HitData& hitData);
  static void ProcessPostureDamage(RE::Actor* aggressor, RE::Actor* victim, float postureDamage);

  static bool IsBlocking(RE::Actor* actor);
  static bool IsTimedBlocking(RE::Actor* actor);

  static void AddTimedBlockListener(std::function<void(RE::Actor*)> callback);

private:
  Block();
  // Rim Combat Block Revert
  // 用于读档的时候清空所有记录
  constexpr static inline std::uint32_t serialType = 'RCBR';

  // 需要锁
  // Actor的格挡开始时间，单位为毫秒
  static inline std::mutex mtx_blockStart;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> blockStartTimes;

  // 高频调用，需要读写锁
  // 限时格挡生效窗口到期时间，单位为毫秒
  static inline std::shared_mutex mtx_timedBlockDuration;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> timedBlockEndTimes;

  // 需要锁
  // 限时格挡事件回调列表
  static inline std::mutex mtx_timedBlockCallback;
  static inline std::vector<std::function<void(RE::Actor*)>> timedBlockCallbacks;
};