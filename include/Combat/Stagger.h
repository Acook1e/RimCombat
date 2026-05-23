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
  // Bool图变量
  // Modern Stagger Lock的硬直恢复状态，
  // RimCombat中调用这个变量来传递是否可以恢复硬直
  constexpr static std::string_view STAGGER_RECOVERABLE = "MSL_IsStaggerRecovery";
  // 图事件，padload用于传递信息
  // TargetSet|level用于设置击中目标的硬直等级为level，level为Level对应的数值
  // TargetEnd用于结束对目标的硬直等级设置和修改，通常在此次攻击命中帧结束时触发
  // Immune|level|Duration用于设置当前的硬直免疫等级为level，level为Level对应的数值，Duration为持续时间，单位为毫秒
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

  static void ProcessWeaponStagger(RE::Actor* aggressor, RE::Actor* victim);
  static void ProcessProjectileStagger(RE::Actor* victim, RE::FormID formID);

  static void StaggerStart(RE::Actor* victim);

  static void TargetSet(RE::Actor* actor, const std::string& payload);
  static void TargetEnd(RE::Actor* actor);
  static void Immune(RE::Actor* actor, const std::string& payload);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Stagger();
  // 用于重置缓存
  // Rim Combat Stagger Cache
  constexpr static inline std::uint32_t serialType = 'RCSC';

  // 无锁，仅初始化时写入，之后只读取
  // 缓存弹射物造成的硬直等级，键为FormID，值为硬直等级
  static inline std::unordered_map<RE::FormID, Level> projectileStagger;

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
  static inline std::unordered_map<RE::Actor*, Level> staggerLevelOnHit;

  // 缓存免疫硬直时间
  static inline std::mutex mtx_immuneCache;
  static inline std::unordered_map<RE::Actor*, std::pair<Level, std::uint64_t>> immuneActors;

  // 缓存受击恢复时间
  static inline std::mutex mtx_recoverTime;
  static inline std::unordered_map<RE::Actor*, std::pair<Level, std::uint64_t>> staggerRecoverTime;
};