#pragma once

#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"
#include "Utils.h"

namespace WeaponArt
{

enum class Rarity : std::uint8_t
{
  Common,
  Rare,
  Epic,
  Legendary
};

class WeaponArtInfo
{
public:
  // 战技的伤害，架势伤害均有payload指定，插件不再区分战技伤害类型

  using AvailableWeaponType = std::uint32_t;
  enum class AvailableWeapon : AvailableWeaponType
  {
    None = 0,  // 作为初始化默认值

    // 武器重量划分

    Light  = 1U << 0,  // 轻型武器，OCF小型分类下的武器
    Normal = 1U << 1,  // 常规武器，OCF单手分类下的武器
    Heavy  = 1U << 2,  // 重型武器，OCF双手分类下的武器

    // 武器攻击划分

    Slash   = 1U << 3,  // 拥有刃面，可以斩击
    Thrust  = 1U << 4,  // 拥有尖端，可以刺击
    Blunt   = 1U << 5,  // 拥有钝面，可以钝击
    Polearm = 1U << 6,  // 拥有长柄，可以进行长柄攻击
    Range   = 1U << 7,  // 可以远程攻击

    // 武器家族 / 子类型划分

    // 原版三类型
    Sword  = 1U << 8,   // 剑类武器
    Axe    = 1U << 9,   // 斧类武器
    Hammer = 1U << 10,  // 锤类武器

    // 模组类型
    Fist   = 1U << 11,  // 拳类武器
    Katana = 1U << 12,  // 武士刀类武器
    Spear  = 1U << 13,  // 枪类武器
    Stick  = 1U << 14,  // 棍类武器
    Whip   = 1U << 15,  // 鞭类武器

    // 远程类型
    Bow      = 1U << 16,  // 弓类武器
    Crossbow = 1U << 17,  // 弩类武器

    // 独特类型
    Unique = 1U << 31,  // 仅特定武器（通过 weapons 列表指定）
  };

  enum class Skill : std::uint8_t
  {
    None,
    OneHanded,
    TwoHanded,
    Archery,
    Block,
    Smithing,
    HeavyArmor,
    LightArmor,
    Pickpocket,
    Lockpicking,
    Sneak,
    Alchemy,
    Speech,
    Alteration,
    Conjuration,
    Destruction,
    Illusion,
    Restoration,
    Enchanting
  };

  struct SpellInfo
  {
    RE::SpellItem* spell;
    float effectiveness;
    float stdMagnitude;
    float factor;
    Skill skill;
    bool selfCast;
  };

  WeaponArtInfo() = default;
  WeaponArtInfo(std::int32_t id, const std::string& name, const std::string& description,
                AvailableWeapon availableWeapon, const std::vector<RE::FormID>& weapons,
                const std::unordered_map<std::uint32_t, SpellInfo>& spells,
                std::uint8_t consumePoint, std::uint8_t unlockLevel, bool needPrepare);

  const std::string& GetName() const { return name; }
  const std::string& GetDescription() const { return description; }
  std::int32_t GetID() const { return id; }

  std::uint8_t GetConsumePoint() const { return consumePoint; }
  std::uint8_t GetUnlockLevel() const { return unlockLevel; }
  bool NeedPrepare() const { return needPrepare; }

  bool IsWeaponAllowed(RE::TESObjectWEAP* weapon) const;

  std::optional<SpellInfo> GetSpellInfo(std::uint32_t hash) const;

private:
  // 战技名称
  std::string name = "";
  // 战技描述
  std::string description = "";
  // 可使用该战技的武器列表，仅当 availableWeapon 为 Unique 时使用
  std::vector<RE::FormID> weapons{};
  // 战技的法术特效名称的hash，映射到SpellInfo
  std::unordered_map<std::uint32_t, SpellInfo> spells{};

  // 战技ID，用于OAR切换战技动画
  std::int32_t id = 0;

  // 战技可用的武器类型
  AvailableWeapon availableWeapon = AvailableWeapon::None;

  // 解锁该战技所需的战技点数
  std::uint8_t consumePoint = 0;
  // 解锁该战技所需的战技等级
  std::uint8_t unlockLevel = 0;

  //  是否使用进入战技状态动画
  // 如果为true，则开启战技时设置
  bool needPrepare = false;
};

class PlayerStat
{
public:
  static PlayerStat& GetSingleton()
  {
    static PlayerStat singleton;
    return singleton;
  }

  static std::uint8_t GetLevel() { return level; }
  static std::uint8_t GetPoint() { return point; }
  static bool IsUnlocked(std::int32_t artID);

  static void AddExp(float value);
  static bool UnlockArt(std::int32_t artID);

private:
  PlayerStat();

  // Weapon Art Player Stat
  constexpr static inline std::uint32_t serialType = 'WAPS';
  static inline float exp                          = 0.0f;  // 当前战技经验
  static inline std::uint8_t level                 = 1;     // 当前战技等级
  static inline std::uint8_t point                 = 3;     // 当前战技点数
  static inline std::unordered_set<std::int32_t> unlockedArts{};
};

class Manager
{
public:
  // 单变量动画变量ID，值为当前武器的战技ID，类型为int
  // 保留0作为非法ID
  constexpr static inline std::string_view ID = "RimCombat_WeaponArtID";
  // 用于标志播放引入动画的变量，类型为bool
  // 在动画开始声明
  // Int图变量
  // 0表示未启用战技系统，1准备中，2表示启用
  // 表示当前的战技系统状态
  constexpr static inline std::string_view STATE = "RimCombat_WeaponArtState";
  // 战技系统事件
  // payload用于传递信息
  // Start|ManaCost|MinMana表示开始使用战技，且需要消耗ManaCost点魔力，至少要MinMana点魔力才能使用
  // 注意Start不要在第0帧调用，MCO/BFCO框架会对第0帧的事件重复触发两次
  // Start的同时会启用RimCombat的耐力系统
  // End标志战技动作结束，设置PERFORMING为0
  // End的同时会结束RimCombat的耐力系统，伤害系统和架势系统的战技相关处理
  // PrepareEnd标志战技准备动画结束，设置STATE为2
  // ToPrepare标志进入战技准备状态，设置STATE为1
  // Cast|SpellName 释放战技的特效，一般为法术，需保证SpellName在当前战技的spells配置中存在且有效
  // SpellName会先转成小写并查表，表项包含effectiveness、stdMagnitude、factor、skill和selfCast
  // stdMagnitude表示25级时的标准强度，最终强度会先乘常驻/临时百分比修正，再乘技能等级曲线
  constexpr static inline std::string_view RIMWEAPONART = "RimWeaponArt";

  // 战技的三种状态：未启用、准备中、已启用
  enum class State : std::uint8_t
  {
    Disable = 0,
    Prepare = 1,
    Enable  = 2,
  };

  enum class Perform : std::int8_t
  {
    None        = 0,  // 没有战技动作
    Eligible    = 1,  // 在战技动画中且满足条件
    Subordinate = 2,  // 在战技动画中但不满足条件
  };

  static Manager& GetSingleton()
  {
    static Manager singleton;
    return singleton;
  }

  static bool IsValidWeaponArtID(std::int32_t artID);

  static std::vector<const WeaponArtInfo*> GetAllWeaponArts();

  static const WeaponArtInfo* GetWeaponArtInfo(std::int32_t artID);

  static void SetWeaponArtInfo(RE::TESObjectWEAP* weapon, std::int32_t artID);

  static std::int32_t GetWeaponArtID(const RE::TESObjectWEAP* weapon);

  static std::int32_t GetActorWeaponArtID(RE::Actor* actor);

  static void UpdateWeaponArt(RE::Actor* actor);

  static Perform GetPerform(RE::Actor* actor);

  static State GetState(RE::Actor* actor);
  static void SetState(RE::Actor* actor, State state);

  static void SwitchWeaponArt(RE::Actor* actor, bool enable);

  static void Start(RE::Actor* actor, const std::string& payload);
  static void End(RE::Actor* actor);
  static void Cast(RE::Actor* actor, const std::string& payload);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);
  static void Interrupt(RE::Actor* actor);

private:
  Manager();

  // 战技ID映射，无需序列化，根据配置信息初始化
  static inline std::unordered_map<std::int32_t, WeaponArtInfo> artMap;

  // RimCombat Weapon Art Info
  constexpr static inline std::uint32_t serialType = 'RWAI';
  static inline std::mutex mtx_infoMap;
  // 武器与战技信息的映射，键为武器FormID，值为对应的战技ID
  // 需要进行序列化，以便保存和加载游戏时保持武器与战技的对应关系
  static inline std::unordered_map<RE::FormID, std::int32_t> infoMap;

  // 缓存当前正在执行战技动作的角色及其状态
  // true表示满足条件，false表示不满足条件
  static inline std::mutex mtx_performCache;
  static inline std::unordered_map<RE::Actor*, bool> actorEligibleCache;
};
}  // namespace WeaponArt
