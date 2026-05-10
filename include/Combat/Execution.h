#pragma once

class Execution
{
public:
  enum class Race : std::uint8_t
  {
    None = 0,
    Human,
  };

  static Execution& GetSingleton()
  {
    static Execution singleton;
    return singleton;
  }

  // 更新可处决Actor列表
  static void Update();

  static Race GetRace(RE::Actor* actor);

  static void SetExecutable(RE::Actor* actor);
  static void TryExecute(RE::Actor* aggressor, RE::Actor* victim);

private:
  Execution();

  // bool行为图变量
  // 表示当前的处决应该位于背部
  // 仅作为推荐，实际处决依然由OAR条件和动画来决定
  constexpr static inline std::string_view BACKSTAB = "RimCombat_Backstab";

  // 可处决的 种族+武器组合
  // 意味某个种族可被人类使用特定类型的武器处决
  // 无需锁定，处决组合只在初始化时设置一次，且之后不再修改
  static inline std::unordered_set<std::uint16_t> availableExcutions;

  // 当前处于可被处决状态的Actor列表
  // 不序列化保存状态
  static inline std::mutex mtx;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> executableActors;
};