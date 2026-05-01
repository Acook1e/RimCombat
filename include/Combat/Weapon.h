#pragma once

// 原版武器判断与模组武器类型支持
namespace Weapon
{
enum class Type : std::uint8_t
{
  // 原版武器类型
  Unarm,        // 空手，不等于拳
  Werewolf,     // 狼人形态空手，特殊处理
  VampireLord,  // 吸血鬼领主形态空手，特殊处理
  Dagger,       // 单手匕首
  Sword,        // 单手剑
  Axe,          // 单手斧
  Mace,         // 单手锤
  GreatSword,   // 双手剑
  GreatAxe,     // 双手斧
  GreatMace,    // 双手锤
  Bow,          // 弓
  Crossbow,     // 弩
  Staff,        // 法杖
  Shield,       // 盾牌，特殊处理，主要用于格挡攻击

  // 模组武器类型
  Fist,          // 单手拳套
  Claw,          // 单手爪
  Rapier,        // 单手细剑
  Katana,        // 单手武士刀
  ShortSpear,    // 单手枪/矛
  Halberd,       // 双手戟
  Spear,         // 双手枪/矛
  Quarterstaff,  // 双手长棍
  GreatKatana,   // 双手武士刀
};

void Initialize();
[[nodiscard]] Type GetWeaponType(RE::TESObjectWEAP* object);
[[nodiscard]] float GetBasePostureDamage(Type type);
[[nodiscard]] float GetBaseStaminaConsumption(Type type);
}  // namespace Weapon