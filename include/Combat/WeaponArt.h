#include "Combat/Weapon.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
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
  enum class AvailableWeapon : std::uint16_t
  {
    Unique = 0,  // 仅特定武器（通过战技信息映射指定）

    // 武器重量划分
    LightWeapon  = 1 << 0,  // 轻型武器（如匕首、短剑）
    NormalWeapon = 1 << 1,  // 常规武器（如单手剑、斧、锤）
    HeavyWeapon  = 1 << 2,  // 重型武器（如双手剑、斧、锤）

    // 武器伤害划分
    SlashWeapon  = 1 << 3,  // 斩击武器（如剑、刀）
    ThrustWeapon = 1 << 4,  // 刺击武器（如长枪、长矛）
    StrikeWeapon = 1 << 5,  // 打击武器（如锤、钝器）
    RangeWeapon  = 1 << 6,  // 远程武器（如弓、弩）

    // 武器类型划分
    Fist    = 1 << 7,   // 拳类武器（如拳套、爪）
    Sword   = 1 << 8,   // 剑类武器（如单手剑、双手剑）
    Machete = 1 << 9,   // 刀类武器（如弯刀、砍刀）
    Axe     = 1 << 10,  // 斧类武器（如单手斧、双手斧）
    Hammer  = 1 << 11,  // 锤类武器（如单手锤、双手锤）
    Spear   = 1 << 12,  // 枪类武器（如长矛、戟）
    Stick   = 1 << 13,  // 棍类武器（如长棍、法杖）
    Bow     = 1 << 14,  // 弓类武器（如弓、弩）
  };

  enum class DamageType : std::uint8_t
  {
    None,    // 无类型伤害
    Slash,   // 斩击
    Thrust,  // 穿刺
    Strike,  // 打击
    Magic,   // 魔法
    Fire,    // 火焰
    Frost,   // 冰霜
    Shock,   // 电击
  };

  WeaponArtInfo() = default;
  WeaponArtInfo(std::int32_t id, const std::string& name,
                const std::string& description, AvailableWeapon availableWeapon,
                const std::vector<RE::FormID>& weapons, DamageType damageType,
                float damageMult, float baseDamage, float postureDamageMult,
                std::uint8_t consumePoint, std::uint8_t unlockLevel);

  const std::string& GetName() const { return name; }
  const std::string& GetDescription() const { return description; }
  const std::int32_t GetID() const { return id; }

  const std::uint8_t GetConsumePoint() const { return consumePoint; }
  const std::uint8_t GetUnlockLevel() const { return unlockLevel; }

  bool IsWeaponAllowed(RE::TESObjectWEAP* weapon) const;

private:
  // 战技名称
  std::string name = "";
  // 战技描述
  std::string description = "";
  // 可使用该战技的武器列表，仅当availableWeapon包含Unique标志时使用
  std::vector<RE::FormID> weapons{};

  // 战技ID，用于OAR切换战技动画
  std::int32_t id = 0;

  // 战技可用的武器类型
  AvailableWeapon availableWeapon = AvailableWeapon::Unique;
  // 战技伤害类型
  DamageType damageType = DamageType::None;

  // 对于无类型伤害，直接乘以伤害倍率
  float damageMult = 1.0f;
  // 对于有类型伤害，应用伤害类型
  float baseDamage = 100.0f;
  // 战技的架势伤害倍率，乘以基础架势伤害
  float postureDamageMult = 1.0f;

  // 解锁该战技所需的战技点数
  std::uint8_t consumePoint = 0;
  // 解锁该战技所需的战技等级
  std::uint8_t unlockLevel = 0;
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

  static void AddExp(float value);
  static bool UnlockArt(const WeaponArtInfo& art);

private:
  PlayerStat();

  // Weapon Art Player Stat
  constexpr static inline std::uint32_t WeaponExp = 'WAPS';
  static inline float exp                         = 0.0f;  // 当前战技经验
  static inline std::uint8_t level                = 1;     // 当前战技等级
  static inline std::uint8_t point                = 0;     // 当前战技点数
  static inline std::unordered_set<std::int32_t> unlockedArts{};
};

class Manager
{
public:
  static Manager& GetSingleton()
  {
    static Manager singleton;
    return singleton;
  }

  static std::vector<const WeaponArtInfo*> GetAllWeaponArts()
  {
    // 一般来说，artMap的内容在游戏运行时不会发生变化
    // 因此可以安全地返回一个静态的包含所有战技信息的向量，避免每次调用都进行构建
    static std::vector<const WeaponArtInfo*> arts;
    if (!arts.empty())
      return arts;
    for (const auto& [id, art] : artMap)
      arts.push_back(&art);
    return arts;
  }

  static const WeaponArtInfo* GetWeaponArtInfo(std::int32_t artID)
  {
    if (artMap.find(artID) == artMap.end())
      return nullptr;
    return &artMap[artID];
  }

  static bool IsValidWeaponArtID(std::int32_t artID)
  {
    return artMap.find(artID) != artMap.end();
  }

  static void SetWeaponArtInfo(RE::TESObjectWEAP* weapon, std::int32_t artID)
  {
    std::lock_guard<std::mutex> lock(mtx);
    auto it = artMap.find(artID);
    if (it == artMap.end())
      return;  // 无效的战技ID，不设置战技信息
    auto& art = it->second;
    if (!art.IsWeaponAllowed(weapon))
      return;  // 武器不符合战技使用条件，不设置战技信息
    infoMap[weapon->GetFormID()] = artID;
  }

  static std::int32_t GetCurrentWeaponArtID(RE::Actor* actor)
  {
    return -865029576;  // 测试用

    if (!actor)
      return 0;

    const auto* left  = actor->GetEquippedObject(true);
    const auto* right = actor->GetEquippedObject(false);

    std::lock_guard<std::mutex> lock(mtx);
    // 优先检查右手武器的战技信息映射
    if (right)
      if (auto it = infoMap.find(right->GetFormID()); it != infoMap.end())
        return it->second;

    if (left)
      if (auto it = infoMap.find(left->GetFormID()); it != infoMap.end())
        return it->second;

    // 左右手皆为空，返回空手状态战技
    return Utils::hash("Unarmed");
  }

  static bool IsEnabled(RE::Actor* actor)
  {
    std::int32_t res = 0;
    if (actor->GetGraphVariableInt(ID, res))
      return res != 0;
    // 如果动画变量不存在，默认返回false
    return false;
  }

  static void EnableWeaponArt(RE::Actor* actor, bool enable)
  {
    std::int32_t id = enable ? GetCurrentWeaponArtID(actor) : 0;
    actor->SetGraphVariableInt(ID, id);
  }

private:
  Manager();

  // 单变量动画变量ID，值为当前武器的战技ID，0表示未启用战技
  const static inline std::string_view ID = "RimCombat_WeaponArtID";

  // 战技ID映射，无需序列化，根据配置信息初始化
  static inline std::unordered_map<std::int32_t, WeaponArtInfo> artMap;

  // RimCombat Weapon Art Info
  constexpr static inline std::uint32_t weaponInfo = 'RWAI';
  static inline std::mutex mtx;
  // 武器与战技信息的映射，键为武器FormID，值为对应的战技ID
  // 需要进行序列化，以便保存和加载游戏时保持武器与战技的对应关系
  static inline std::unordered_map<RE::FormID, std::int32_t> infoMap;
};
}  // namespace WeaponArt
