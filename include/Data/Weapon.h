#pragma once

#include "Core/Settings.h"

// 原版武器判断与模组武器类型支持
namespace Weapon
{

using EnumType = Settings::WeaponEnumType;

enum class Type : EnumType
{
  // 无类型，主要用于错误处理
  None = 0,
  // 空手类型
  Unarm       = 0x01,  // 空手，不等于拳
  Werewolf    = 0x02,  // 狼人形态空手，特殊处理
  Werebear    = 0x03,  // 熊人形态空手，特殊处理
  VampireLord = 0x04,  // 吸血鬼领主形态空手，特殊处理

  // 近战类型
  // 基于大类和OCF子类型划分

  // 单手类型
  // 小型双面刃类
  Dagger = 0x05,  // 单手匕首
  Claw   = 0x06,  // 单手爪

  // 双面刃类
  Sword  = 0x07,  // 单手剑
  Rapier = 0x08,  // 单手细剑
  Katana = 0x09,  // 单手武士刀

  // 单面刃类
  WarAxe = 0x0A,  // 单手斧 暂且包含 Hatchet-手斧

  // 钝器类
  Mace   = 0x0B,  // 单手锤 暂且包含 Maul-狼牙棒 Club-棍棒
  Cestus = 0x0C,  // 单手拳套
  Whip   = 0x0D,  // 单手鞭

  // 单手长柄类
  ShortSpear = 0x0E,  // 单手枪/矛 暂且包含 ShortPike-短枪/矛

  // 双手类型
  // 双手双面刃类
  GreatSword  = 0x0F,  // 双手剑
  GreatKatana = 0x10,  // 双手武士刀

  // 双手单面刃类
  BattleAxe = 0x11,  // 双手斧

  // 双手钝器类
  WarHammer    = 0x12,  // 双手锤 暂且包含 LongMace-长柄锤 GreatClub-大型棍棒
  Quarterstaff = 0x13,  // 双手长棍

  // 双手长柄类
  Glaive  = 0x14,  // 双手长柄单刃刀
  Pike    = 0x15,  // 双手枪/矛 暂且包含 Spear-长枪 Trident-三叉戟
  Halberd = 0x16,  // 双手斧枪

  // 远程类型
  Bow      = 0x17,  // 弓
  Crossbow = 0x18,  // 弩

  // 特殊类型
  Staff  = 0x19,  // 法杖
  Shield = 0x1A,  // 盾牌，特殊处理，主要用于格挡攻击
  Torch  = 0x1B,  // 火把，特殊处理，主要用于点燃目标
};

void Initialize();
[[nodiscard]] bool IsUnarmed(Type type);

[[nodiscard]] Type GetActorEquipmentType(RE::Actor* actor, bool leftHand = false);
[[nodiscard]] Type GetWeaponType(RE::Actor* actor, RE::TESObjectWEAP* object);
[[nodiscard]] Type GetBlockType(RE::Actor* actor);

[[nodiscard]] float GetBasePostureDamage(Type type);
[[nodiscard]] float GetBasePoiseDamage(Type type);
[[nodiscard]] float GetBaseStaminaConsumption(Type type);
[[nodiscard]] float GetBlockStrength(Type type);
[[nodiscard]] float GetBaseExecutionMultiplier(RE::Actor* actor);
}  // namespace Weapon