#pragma once

// 原版武器判断与模组武器类型支持
namespace Weapon
{
enum class Type : std::uint8_t
{
  None = 0,  // 无类型，主要用于错误处理
  // 原版武器类型
  Unarm       = 0x01,  // 空手，不等于拳
  Werewolf    = 0x02,  // 狼人形态空手，特殊处理
  VampireLord = 0x03,  // 吸血鬼领主形态空手，特殊处理
  Dagger      = 0x04,  // 单手匕首
  Sword       = 0x05,  // 单手剑
  Axe         = 0x06,  // 单手斧
  Mace        = 0x07,  // 单手锤
  GreatSword  = 0x08,  // 双手剑
  GreatAxe    = 0x09,  // 双手斧
  GreatMace   = 0x0A,  // 双手锤
  Bow         = 0x0B,  // 弓
  Crossbow    = 0x0C,  // 弩
  Staff       = 0x0D,  // 法杖
  Shield      = 0x0E,  // 盾牌，特殊处理，主要用于格挡攻击
  Torch       = 0x0F,  // 火把，特殊处理，主要用于点燃目标

  // 模组武器类型
  Fist         = 0x10,  // 单手拳套
  Claw         = 0x11,  // 单手爪
  Rapier       = 0x12,  // 单手细剑
  Katana       = 0x13,  // 单手武士刀
  ShortSpear   = 0x14,  // 单手枪/矛
  Halberd      = 0x15,  // 双手戟
  Spear        = 0x16,  // 双手枪/矛
  Quarterstaff = 0x17,  // 双手长棍
  GreatKatana  = 0x18,  // 双手武士刀
};

void Initialize();
[[nodiscard]] Type GetActorEquipmentType(RE::Actor* actor, bool leftHand = false);
[[nodiscard]] Type GetWeaponType(RE::TESObjectWEAP* object);
[[nodiscard]] Type GetBlockType(RE::Actor* actor);
[[nodiscard]] float GetBasePostureDamage(Type type);
[[nodiscard]] float GetBaseStaminaConsumption(Type type);
[[nodiscard]] float GetBlockStrength(Type type);
[[nodiscard]] float GetBaseExecutionMultiplier(RE::Actor* actor);
}  // namespace Weapon