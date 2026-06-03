#pragma once

#include "Core/Settings.h"

// 原版武器判断与模组武器类型支持
namespace Weapon
{

using EnumType = Settings::WeaponEnumType;

// 仅针对人类或者可以使用武器的种族
// 对于生物使用Race判断
enum class Type : EnumType
{
  // 无类型，主要用于错误处理
  None = 0,
  // 空手类型
  Unarm = 1,  // 空手，不等于拳
  Spell = 2,  // 法术

  // 近战类型
  // 基于大类和OCF子类型划分

  // 单手类型
  // 小型双面刃类
  Dagger = 10,  // 单手匕首
  Claw   = 11,  // 单手爪

  // 双面刃类
  Sword  = 20,  // 单手剑
  Rapier = 21,  // 单手细剑
  Katana = 22,  // 单手武士刀

  // 单面刃类
  WarAxe = 30,  // 单手斧 暂且包含 Hatchet-手斧

  // 钝器类
  Mace   = 40,  // 单手锤 暂且包含 Maul-狼牙棒 Club-棍棒
  Cestus = 41,  // 单手拳套
  Whip   = 42,  // 单手鞭

  // 单手长柄类
  ShortSpear = 50,  // 单手枪/矛 暂且包含 ShortPike-短枪/矛

  // 双手类型
  // 双手双面刃类
  GreatSword  = 60,  // 双手剑
  GreatKatana = 61,  // 双手武士刀

  // 双手单面刃类
  BattleAxe = 70,  // 双手斧

  // 双手钝器类
  WarHammer    = 80,  // 双手锤 暂且包含 LongMace-长柄锤 GreatClub-大型棍棒
  Quarterstaff = 81,  // 双手长棍

  // 双手长柄类
  Glaive  = 90,  // 双手长柄单刃刀
  Pike    = 91,  // 双手枪/矛 暂且包含 Spear-长枪 Trident-三叉戟
  Halberd = 92,  // 双手斧枪

  // 远程类型
  Bow      = 100,  // 弓
  Crossbow = 101,  // 弩

  // 特殊类型
  Staff  = 200,  // 法杖
  Shield = 201,  // 盾牌，特殊处理，主要用于格挡攻击
  Torch  = 202,  // 火把，特殊处理，主要用于点燃目标
};

void Initialize();

[[nodiscard]] Type GetActorEquipmentType(RE::Actor* actor, bool leftHand = false);
[[nodiscard]] Type GetWeaponType(RE::TESObjectWEAP* weapon);
[[nodiscard]] Type GetBlockType(RE::Actor* actor);

[[nodiscard]] float GetBasePostureDamage(Type type);
[[nodiscard]] float GetBasePoiseDamage(Type type);
[[nodiscard]] float GetBaseStaminaConsumption(Type type);
[[nodiscard]] float GetBlockStrength(Type type);
[[nodiscard]] float GetBaseExecutionMultiplier(RE::Actor* actor);
}  // namespace Weapon