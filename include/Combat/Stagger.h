#pragma once

class Stagger
{
public:
  enum class Level : std::uint8_t
  {
    // Rim Poise系统的硬直等级

    None,
    Small,
    Medium,
    Large,

    // Largest 等级，默认情况下是架势崩溃状态专用的硬直
    Largest,
    Execution = Largest,

    // 根据攻击类型， RimCombat补充的额外级别

    Knockdown,  // 砸趴
    Strikefly,  // 挑飞
    Knockaway,  // 击飞

    Total
  };

  // Int图变量
  // 在硬直位于Largest，根据这个判断是否处于RimCombat的额外硬直等级中
  constexpr static std::string_view STAGGER_LEVEL = "RimCombat_StaggerLevel";
  // Int图变量
  // 在执行硬直之前，根据这个判断是否免疫硬直
  constexpr static std::string_view STAGGER_IMMUNE = "RimCombat_StaggerImmune";
  // 图事件，padload用于传递信息
  // Set|level用于设置当前的硬直等级为level，level为Level对应的数值
  // End表示退出RimCombat的硬直系统，恢复原版的硬直处理
  // Immune|level|Duration用于设置当前的硬直免疫等级为level，level为Level对应的数值，Duration为持续时间，单位为毫秒
  // TargetSet|level用于设置击中目标的硬直等级为level，level为Level对应的数值
  // TargetModify|level用于修改击中目标的硬直等级，level为要修改的等级值，正数表示增加等级，负数表示降低等级
  // 如果Set和Modify同时存在，则只会Set目标的硬直等级，Modify会被忽略
  // TargetEnd用于结束对目标的硬直等级设置和修改，通常在此次攻击命中帧结束时触发
  constexpr static std::string_view RIMSTAGGER = "RimStagger";

  static Stagger& GetSingleton()
  {
    static Stagger singleton;
    return singleton;
  }

  static void Update();

  static float LevelToMagnitude(Level level);
  static Level MagnitudeToLevel(float magnitude);

  static float GetStaggerMagnitude(RE::Actor* actor);
  static void SetStaggerMagnitude(RE::Actor* actor, Level level);

  static Level GetStaggerLevel(RE::Actor* actor);
  static void SetStaggerLevel(RE::Actor* actor, Level level);

  static bool IsImmune(RE::Actor* actor);
  static Level GetImmuneLevel(RE::Actor* actor);
  static void SetImmuneLevel(RE::Actor* actor, Level level);

  // MaxsuPoise的硬直计算和写回发生在TryStagger
  // 确保调用时已经处理完攻击了
  // Modern Stagger Lock的硬直等级计算是在NotifyAnimationGraph中进行的
  // 但我们的硬直等级处理在PerformAction中，早于NotifyAnimationGraph，因此需要在这里应用最终的硬直等级
  // 返回true表示这次硬直处理被RimCombat接管了，返回false表示不处理，交给原版
  static void ProcessStagger(RE::Actor* aggressor, RE::Actor* victim);

  static void TargetSet(RE::Actor* actor, const std::string& payload);
  static void TargetModify(RE::Actor* actor, const std::string& payload);
  static void TargetEnd(RE::Actor* actor);
  static void Immune(RE::Actor* actor, const std::string& payload);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Stagger();
  // 用于重置缓存
  // Rim Combat Stagger Cache
  constexpr static inline std::uint32_t serialType = 'RCSC';

  // 硬直等级取值范围
  // 因为Modern Stagger Lock没有把这些设置写入游戏
  // 手动从ini读取这些值
  static inline std::unordered_map<Level, float> staggerMagnitudeMap{
      {Level::None, 0.0f},   {Level::Small, 0.25f},  {Level::Medium, 0.5f},
      {Level::Large, 0.75f}, {Level::Largest, 1.0f},
  };

  // 由于动画事件的发送早于受击处理
  // 因此缓存在动画事件的数据，在受击处理时再处理
  static inline std::mutex mtx_targetCache;
  static inline std::unordered_map<RE::Actor*, Level> setTargetLevelMap;
  static inline std::unordered_map<RE::Actor*, std::int8_t> modifyTargetLevelMap;

  // 缓存免疫硬直时间
  static inline std::mutex mtx_immuneCache;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> immuneActors;
};