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

    // Largest 等级，在这个等级之上的均是特殊硬直等级
    Largest,

    // RimCombat硬直系统的额外等级，单次只生效最初的触发的等级，不会覆盖
    // 若位于格挡状态，则不会触发这些硬直等级，但会根据硬直等级产生额外的耐力消耗

    Knockaway = 100,  // 击飞
    Knockdown = 101,  // 砸趴
    Strikefly = 102,  // 挑飞

    // 无法被任何状态免疫的特殊等级，通常用于特定敌人或特定攻击的特殊效果

    GuardBreak = 200,  // 防御崩溃，由弹反或者格挡耐力消耗到0触发

    // 默认处决为最高优先级的硬直，会覆盖其他所有硬直等级，且无法被免疫

    Execution     = 253,  // 处决，是被处决者的受击动画
    ExecutionBack = 254,  // 背部处决
    PostureBreak  = 255,  // 架势崩溃，由架势值被打破触发
  };

  // Int图变量
  // 在硬直位于Largest，根据这个判断是否处于RimCombat的额外硬直等级中
  constexpr static std::string_view STAGGER_LEVEL = "RimCombat_StaggerLevel";
  // Bool图变量
  // Modern Stagger Lock的硬直恢复状态，
  // RimCombat中调用这个变量来传递是否可以恢复硬直
  constexpr static std::string_view STAGGER_RECOVERABLE = "MSL_IsStaggerRecovery";
  // 图事件，padload用于传递信息
  // TargetSet|level 用于设置击中目标的硬直等级为level，level为Level对应的数值
  // TargetEnd 用于结束对目标的硬直等级设置和修改，通常在此次攻击命中帧结束时触发
  // Immune|Duration 用于设置硬直免疫
  // 在此期间可以免疫任何可被免疫的硬直等级，Duration为持续时间，单位为毫秒
  // Recoverable用于设置在这个时间后可以从硬直中恢复，通常在硬直动画末尾触发
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

  static Level IsInStagger(RE::Actor* actor);
  static Level GetStaggerLevel(RE::Actor* actor);
  static void SetStaggerLevel(RE::Actor* actor, Level level);

  static std::uint64_t GetRecoverTime(RE::Actor* actor, Level level);

  static bool IsImmune(RE::Actor* actor);

  static void ProcessWeaponStagger(RE::Actor* aggressor, RE::Actor* victim);
  static void ProcessProjectileStagger(RE::Actor* victim, RE::FormID formID);

  static void StaggerStart(RE::Actor* victim);

  static void TargetSet(RE::Actor* actor, const std::string& payload);
  static void TargetEnd(RE::Actor* actor);
  static void Immune(RE::Actor* actor, const std::string& payload);
  static void Recoverable(RE::Actor* actor);

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
  static inline std::mutex mtx_levelCache;
  static inline std::unordered_map<RE::Actor*, Level> staggerLevelOnAttack;

  // 缓存免疫硬直时间
  static inline std::mutex mtx_immuneCache;
  static inline std::unordered_map<RE::Actor*, std::uint64_t> immuneActors;

  // 缓存受击恢复时间
  static inline std::mutex mtx_recover;
  struct RecoveryData
  {
    std::uint64_t recoverTime;  // 恢复时间点，单位为毫秒时间戳
    Level current;              // 当前的硬直等级
  };
  static inline std::unordered_map<RE::Actor*, RecoveryData> staggerRecovery;
};