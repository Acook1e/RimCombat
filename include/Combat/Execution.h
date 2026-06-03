#pragma once

#include "Data/Race.h"
#include "Data/Weapon.h"

class Execution
{
public:
  static Execution& GetSingleton()
  {
    static Execution singleton;
    return singleton;
  }

  // 更新可处决Actor列表
  static void Update();

  static bool IsExecutable(RE::Actor* actor);
  static void EnterExecutable(RE::Actor* actor);
  static void ExitExecutable(RE::Actor* actor);

  static RE::Actor* FindExecutableTarget(RE::Actor* aggressor);
  static bool TryExecute(RE::Actor* aggressor, RE::Actor* victim);

  static RE::Actor* GetExecutingAggressor(RE::Actor* victim);
  static bool IsExecutingVictim(RE::Actor* victim);
  static void ExecutionEnd(RE::Actor* aggressor);

  using ExecutionStartCallback =
      std::function<void(RE::Actor* aggressor, RE::Actor* victim, bool back)>;
  static void AddExecutionStartListener(ExecutionStartCallback callback);

  static void Damage(RE::Actor* victim, const std::string& payload);
  static void SetDistance(RE::Actor* victim, const std::string& payload);
  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Execution();

  // Int行为图变量
  // 表示处决者的武器类型，以及受害者对应的受击动画
  // 为0表示None，表示不处于处决状态
  constexpr static inline std::string_view EXECUTOR_WEAPON = "RimCombat_ExecutorWeapon";
  // Int行为图变量
  // 表示受害者的种族类型，以及处决者对应的处决动画
  // 为0表示None，表示不处于处决状态
  constexpr static inline std::string_view VICTIM_RACE = "RimCombat_VictimRace";
  // 行为事件
  // 其payload表示处决相关的的事件
  // Damage|Multiplier 表示进行一次真实伤害结算，Multiplier为一个数字，表示伤害的倍率
  // SetDistance|Value 表示调整处决双方的距离，Value为一个数字，表示距离的数值
  // End表示结束处决状态，触发受害者退出处决状态的逻辑
  constexpr static inline std::string_view EXECUTION_END = "RimExecution";

  // 无锁，处决组合只在初始化时设置一次，且之后不再修改
  // 意味某个种族可被人类使用特定类型的武器处决
  static inline std::unordered_map<Race::Type, std::vector<Weapon::Type>> availableExcutions;

  // 需要锁
  // 当前处于可被处决状态的Actor列表，值为处决状态到期时间
  // 不序列化保存状态
  static inline std::mutex mtx_executable;
  static inline std::unordered_set<RE::Actor*> executableActors;

  // 需要锁
  // 保存受害者 -> 处决者，用于在处决过程中持续处理伤害等逻辑
  // 不序列化保存状态
  static inline std::mutex mtx_executing;
  static inline std::unordered_map<RE::Actor*, RE::Actor*> executingActors;

  // 需要锁
  // 订阅处决事件开始时的callback
  static inline std::mutex mtx_startListeners;
  static inline std::vector<ExecutionStartCallback> executionStartListeners;
};