#pragma once

#include "Combat/Stagger.h"

class Block
{
public:
  // 图事件
  // GP|Duration|Level|AutoAttack|NextAttack
  // 进入可精确格挡状态，在这个状态内受击会给予攻击者一个硬直，并且判断为限时格挡
  // 硬直等级由Level决定，Duration为持续时间，AutoAttack表示成功GP后是否自动反击，NextAttack表示GP成功的下一次的攻击段数
  // NextAttack为N2表示自动反击使用第二段轻击，P4表示使用第四段重击，0表示不修改攻击段数
  // 若AutoAttack为true但NextAttack为0，则不进行自动反击
  // Parry|Duration 进入弹反状态，在这个状态内受击会给予攻击者一个GuardBreak级别的硬直
  constexpr static inline std::string_view RIMBLOCK = "RimBlock";

  static Block& GetSingleton()
  {
    static Block singleton;
    return singleton;
  }

  // 挂在主线程更新中，定期清理过期的格挡时间记录
  static void Update();

  static void StartBlock(RE::Actor* actor);
  static void EndBlock(RE::Actor* actor);

  static void ProcessBlock(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData);
  static void ProcessPostureDamage(RE::Actor* aggressor, RE::Actor* victim, float postureDamage);

  static bool IsBlocking(RE::Actor* actor);
  static bool IsTimedBlocking(RE::Actor* actor);

  static void GP(RE::Actor* actor, const std::string& payload);
  static void Parry(RE::Actor* actor, const std::string& payload);

  static void ParsePayload(RE::Actor* actor, const std::string& payload);

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
  // 缓存GP相关的数据
  struct GPData
  {
    std::uint64_t endTime;
    Stagger::Level level;
    bool autoAttack;
    bool isPowerAttack;
    std::uint8_t nextAttack;
  };
  static inline std::mutex mtx_gpData;
  static inline std::unordered_map<RE::Actor*, GPData> gpData;

  // 需要锁
  // 缓存弹反结束时间，单位为毫秒
  static inline std::mutex mtx_parry;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> parryEndTimes;

  // 需要锁
  // 限时格挡事件回调列表
  static inline std::mutex mtx_timedBlockCallback;
  static inline std::vector<std::function<void(RE::Actor*)>> timedBlockCallbacks;
};