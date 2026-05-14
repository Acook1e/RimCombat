#pragma once

class Execution
{
public:
  enum class Race : std::uint8_t
  {
    None  = 0,
    Human = 0x01,
  };

  static Execution& GetSingleton()
  {
    static Execution singleton;
    return singleton;
  }

  // 更新可处决Actor列表
  static void Update();

  static Race GetRace(RE::Actor* actor);

  static bool IsExecutable(RE::Actor* actor);
  static void EnterExecutable(RE::Actor* actor);
  static void ExitExecutable(RE::Actor* actor);

  static RE::Actor* FindExecutableTarget(RE::Actor* aggressor);
  static bool TryExecute(RE::Actor* aggressor, RE::Actor* victim);

  static RE::Actor* GetExecutionVictim(RE::Actor* aggressor);
  static void ExecutionEnd(RE::Actor* aggressor);

  using ExecutionStartCallback =
      std::function<void(RE::Actor* aggressor, RE::Actor* victim, bool back)>;
  static void AddExecutionStartListener(ExecutionStartCallback callback);

  static void CalculateExecutionDamage(RE::Actor* aggressor, RE::Actor* victim);

private:
  Execution();
  static void LockActor(RE::Actor* actor);
  static void UnlockActor(RE::Actor* actor);

  // book行为图变量
  // 表示当前的Actor处于可被处决状态
  constexpr static inline std::string_view EXECUTABLE = "RimCombat_Executable";
  // Int行为图变量
  // OAR根据这个变量来判断播放哪个处决动画
  // 为0表示无状态，否则表示当前武器+种族的处决组合ID
  constexpr static inline std::string_view EXECUTION_FLAG = "RimCombat_ExecutionFlag";
  // 行为事件
  // 用于标志处决结束，解除处决状态
  constexpr static inline std::string_view EXECUTION_END = "RimCombat_ExecutionEnd";

  // 无锁，处决组合只在初始化时设置一次，且之后不再修改
  // 可处决的 种族+武器组合 映射初始距离
  // 意味某个种族可被人类使用特定类型的武器处决
  static inline std::unordered_map<std::uint16_t, float> availableExcutions;

  // 需要锁
  // 当前处于可被处决状态的Actor列表
  // 不序列化保存状态
  static inline std::mutex mtx_executable;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> executableActors;

  // 需要锁
  // 处于处决状态的Actor及其对应的处决者，用于在处决过程中持续处理伤害等逻辑
  // 不序列化保存状态
  static inline std::mutex mtx_executing;
  static inline std::unordered_map<RE::Actor*, RE::Actor*> executingActors;

  // 需要锁
  // 订阅处决事件开始时的callback
  static inline std::mutex mtx_startListeners;
  static inline std::vector<ExecutionStartCallback> executionStartListeners;
};