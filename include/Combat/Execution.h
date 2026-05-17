#pragma once

class Execution
{
public:
  enum class Race : std::uint8_t
  {
    // 野猪，蓝客灵骑乘野猪 具有相同行为图
    // 查鲁斯，查鲁斯收割者具有相同行为图
    // 狗，狐狸，狼具有相同行为图
    // 蜘蛛，巨型蜘蛛，大型蜘蛛具有相同行为图
    // 熊人，狼人形态具有相同行为图

    None             = 0,
    Human            = 0x01,
    AshHopper        = 0x02,
    Bear             = 0x03,
    Boar             = 0x04,
    BoarMounted      = 0x05,
    Chaurus          = 0x06,
    ChaurusHunter    = 0x07,
    ChaurusReaper    = 0x08,
    Chicken          = 0x09,
    Cow              = 0x0A,
    Deer             = 0x0B,
    Dog              = 0x0C,
    Dragon           = 0x0D,
    DragonPriest     = 0x0E,
    Draugr           = 0x0F,
    DwarvenBallista  = 0x10,
    DwarvenCenturion = 0x11,
    DwarvenSphere    = 0x12,
    DwarvenSpider    = 0x13,
    Falmer           = 0x14,
    FlameAtronach    = 0x15,
    Fox              = 0x16,
    FrostAtronach    = 0x17,
    Gargoyle         = 0x18,
    Giant            = 0x19,
    GiantSpider      = 0x1A,
    Goat             = 0x1B,
    Hagraven         = 0x1C,
    Hare             = 0x1D,
    Horker           = 0x1E,
    Horse            = 0x1F,
    IceWraith        = 0x20,
    LargeSpider      = 0x21,
    Lurker           = 0x22,
    Mammoth          = 0x23,
    Mudcrab          = 0x24,
    Netch            = 0x25,
    Riekling         = 0x26,
    Sabrecat         = 0x27,
    Seeker           = 0x28,
    Skeever          = 0x29,
    Slaughterfish    = 0x2A,
    Spider           = 0x2B,
    Spriggan         = 0x2C,
    StormAtronach    = 0x2D,
    Troll            = 0x2E,
    VampireLord      = 0x2F,
    Werebear         = 0x30,
    Werewolf         = 0x31,
    Wisp             = 0x32,
    Wispmother       = 0x33,
    Wolf             = 0x34,
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

  static RE::Actor* GetExecutingAggressor(RE::Actor* victim);
  static bool IsExecutingVictim(RE::Actor* victim);
  static void ApplyExecutionDamage(RE::Actor* victim, std::string payload);
  static void ExecutionEnd(RE::Actor* aggressor);

  using ExecutionStartCallback =
      std::function<void(RE::Actor* aggressor, RE::Actor* victim, bool back)>;
  static void AddExecutionStartListener(ExecutionStartCallback callback);

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
  // 其payload表示处决相关的的事件
  // damage|xxx表示进行一次真实伤害结算，xxx为一个数字，表示伤害的倍率
  // end表示处决结束
  constexpr static inline std::string_view EXECUTION_END = "RimExecution";

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
  // 保存受害者 -> 处决者，用于在处决过程中持续处理伤害等逻辑
  // 不序列化保存状态
  static inline std::mutex mtx_executing;
  static inline std::unordered_map<RE::Actor*, RE::Actor*> executingActors;

  // 需要锁
  // 订阅处决事件开始时的callback
  static inline std::mutex mtx_startListeners;
  static inline std::vector<ExecutionStartCallback> executionStartListeners;
};