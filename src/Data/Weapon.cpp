#include "Data/Weapon.h"

#include "Core/Settings.h"

#include "magic_enum/magic_enum.hpp"

namespace Weapon
{

inline Type DebugInfo(RE::TESObjectWEAP* weapon, Type type)
{
  logger::warn("Weapon::GetWeaponType: Failed to determine weapon type for {}, formID: {:X}, "
               "Fallback type: {}",
               weapon->GetName(), weapon->GetFormID(), magic_enum::enum_name(type));

  auto keywords = weapon->GetKeywords();
  logger::info("Weapon {} has {}", weapon->GetName(), keywords.size());
  for (auto& keyword : keywords) {
    auto editorID = keyword->GetFormEditorID();
    logger::info("\t - {}", editorID ? editorID : "None");
  }
  return type;
};

// 原版
static RE::BGSKeyword* warhammerKeyword = nullptr;
static RE::TESRace* werewolfRace        = nullptr;
static RE::TESRace* werebearRace        = nullptr;
static RE::TESRace* vampireLordRace     = nullptr;

// 直接基于OCF框架
bool OCF_Installed = false;

// 类型关键词
// 区分大类型，减少对每把武器的关键词查询次数
// 空手类
static RE::BGSKeyword* OCF_Unarmed = nullptr;
// 小型单手刀剑类
static RE::BGSKeyword* OCF_SmallBladed1H = nullptr;
// 单面刃类
static RE::BGSKeyword* OCF_Bladed1H = nullptr;
// 单面刃类
static RE::BGSKeyword* OCF_Edged1H = nullptr;
// 钝器类
static RE::BGSKeyword* OCF_Blunt1H = nullptr;
// 单手长柄类
static RE::BGSKeyword* OCF_Pole1H = nullptr;

// 双手单面刃类
static RE::BGSKeyword* OCF_Bladed2H = nullptr;
// 双手单面刃类
static RE::BGSKeyword* OCF_Edged2H = nullptr;
// 双手钝器类
static RE::BGSKeyword* OCF_Blunt2H = nullptr;
// 双手长柄类
static RE::BGSKeyword* OCF_Pole2H = nullptr;

// 武器关键词
// 一般是直接映射类型
static RE::BGSKeyword* OCF_Dagger1H = nullptr;  // Dagger
static RE::BGSKeyword* OCF_Claw1H   = nullptr;  // Claw

static RE::BGSKeyword* OCF_Sword1H  = nullptr;  // Sword
static RE::BGSKeyword* OCF_Rapier1H = nullptr;  // Rapier
static RE::BGSKeyword* OCF_Katana1H = nullptr;  // Katana

static RE::BGSKeyword* OCF_WarAxe1H  = nullptr;  // WarAxe
static RE::BGSKeyword* OCF_Hatchet1H = nullptr;  // Hatchet

static RE::BGSKeyword* OCF_Mace1H   = nullptr;  // Mace
static RE::BGSKeyword* OCF_Hammer1H = nullptr;  // Maul
static RE::BGSKeyword* OCF_Club1H   = nullptr;  // Club
static RE::BGSKeyword* OCF_Cestus1H = nullptr;  // Cestus
static RE::BGSKeyword* OCF_Whip1H   = nullptr;  // Whip

static RE::BGSKeyword* OCF_Pike1H  = nullptr;  // ShortPike
static RE::BGSKeyword* OCF_Spear1H = nullptr;  // ShortSpear

static RE::BGSKeyword* OCF_Sword2H  = nullptr;  // GreatSword
static RE::BGSKeyword* OCF_Katana2H = nullptr;  // GreatKatana

static RE::BGSKeyword* OCF_BattleAxe2H = nullptr;  // BattleAxe

static RE::BGSKeyword* OCF_WarHammer2H    = nullptr;  // WarHammer
static RE::BGSKeyword* OCF_Mace2H         = nullptr;  // LongMace
static RE::BGSKeyword* OCF_Club2H         = nullptr;  // GreatClub
static RE::BGSKeyword* OCF_Quarterstaff2H = nullptr;  // Quarterstaff

static RE::BGSKeyword* OCF_Glaive2H  = nullptr;  // Glaive
static RE::BGSKeyword* OCF_Pike2H    = nullptr;  // Pike
static RE::BGSKeyword* OCF_Spear2H   = nullptr;  // Spear
static RE::BGSKeyword* OCF_Trident2H = nullptr;  // Trident
static RE::BGSKeyword* OCF_Halberd2H = nullptr;  // Halberd

void Initialize()
{
  auto dataHandler = RE::TESDataHandler::GetSingleton();
  if (!dataHandler) {
    logger::error("Weapon::Initialize: Failed to get TESDataHandler");
    return;
  }

  // 原版内容不做检查，因为它们的存在是毋庸置疑的
  // 三种特殊变身的种族，分别对应狼人、熊人和吸血鬼领主的空手类型
  werewolfRace    = RE::TESForm::LookupByID<RE::TESRace>(0xCDD84);
  werebearRace    = dataHandler->LookupForm<RE::TESRace>(0x1E17B, "Dragonborn.esm");
  vampireLordRace = dataHandler->LookupForm<RE::TESRace>(0x283A, "Dawnguard.esm");

  // 原版双手斧和双手锤都使用了同一个WEAPON_TYPE，因此需要通过关键字来区分重型斧和重型锤
  warhammerKeyword = RE::TESForm::LookupByID<RE::BGSKeyword>(0x6D930);

  // 检测模组加载情况
  OCF_Installed = dataHandler->LookupModByName("OCF.esp") != nullptr;
  if (!OCF_Installed) {
    logger::info("Weapon::Initialize: OCF.esp not found, mod support disabled");
    return;
  }

  // 录入关键词
  OCF_Unarmed       = dataHandler->LookupForm<RE::BGSKeyword>(0x82D, "OCF.esp");
  OCF_SmallBladed1H = dataHandler->LookupForm<RE::BGSKeyword>(0x887, "OCF.esp");
  OCF_Bladed1H      = dataHandler->LookupForm<RE::BGSKeyword>(0x884, "OCF.esp");
  OCF_Edged1H       = dataHandler->LookupForm<RE::BGSKeyword>(0x886, "OCF.esp");
  OCF_Blunt1H       = dataHandler->LookupForm<RE::BGSKeyword>(0x885, "OCF.esp");
  OCF_Pole1H        = dataHandler->LookupForm<RE::BGSKeyword>(0x824, "OCF.esp");

  OCF_Bladed2H = dataHandler->LookupForm<RE::BGSKeyword>(0x881, "OCF.esp");
  OCF_Edged2H  = dataHandler->LookupForm<RE::BGSKeyword>(0x883, "OCF.esp");
  OCF_Blunt2H  = dataHandler->LookupForm<RE::BGSKeyword>(0x882, "OCF.esp");
  OCF_Pole2H   = dataHandler->LookupForm<RE::BGSKeyword>(0x842, "OCF.esp");

  OCF_Dagger1H = dataHandler->LookupForm<RE::BGSKeyword>(0x802, "OCF.esp");
  OCF_Claw1H   = dataHandler->LookupForm<RE::BGSKeyword>(0x777776, "Update.esm");

  OCF_Sword1H  = dataHandler->LookupForm<RE::BGSKeyword>(0x810, "OCF.esp");
  OCF_Rapier1H = dataHandler->LookupForm<RE::BGSKeyword>(0x777774, "Update.esm");
  OCF_Katana1H = dataHandler->LookupForm<RE::BGSKeyword>(0x777779, "Update.esm");

  OCF_WarAxe1H  = dataHandler->LookupForm<RE::BGSKeyword>(0x818, "OCF.esp");
  OCF_Hatchet1H = dataHandler->LookupForm<RE::BGSKeyword>(0x815, "OCF.esp");

  OCF_Mace1H   = dataHandler->LookupForm<RE::BGSKeyword>(0x821, "OCF.esp");
  OCF_Hammer1H = dataHandler->LookupForm<RE::BGSKeyword>(0x820, "OCF.esp");
  OCF_Club1H   = dataHandler->LookupForm<RE::BGSKeyword>(0x81E, "OCF.esp");
  OCF_Cestus1H = dataHandler->LookupForm<RE::BGSKeyword>(0x81D, "OCF.esp");
  OCF_Whip1H   = dataHandler->LookupForm<RE::BGSKeyword>(0x777778, "Update.esm");

  OCF_Pike1H  = dataHandler->LookupForm<RE::BGSKeyword>(0x82A, "OCF.esp");
  OCF_Spear1H = dataHandler->LookupForm<RE::BGSKeyword>(0x82B, "OCF.esp");

  OCF_Sword2H  = dataHandler->LookupForm<RE::BGSKeyword>(0x867, "OCF.esp");
  OCF_Katana2H = dataHandler->LookupForm<RE::BGSKeyword>(0x82E, "OCF.esp");

  OCF_BattleAxe2H = dataHandler->LookupForm<RE::BGSKeyword>(0x837, "OCF.esp");

  OCF_WarHammer2H    = dataHandler->LookupForm<RE::BGSKeyword>(0x841, "OCF.esp");
  OCF_Mace2H         = dataHandler->LookupForm<RE::BGSKeyword>(0x840, "OCF.esp");
  OCF_Club2H         = dataHandler->LookupForm<RE::BGSKeyword>(0x83F, "OCF.esp");
  OCF_Quarterstaff2H = dataHandler->LookupForm<RE::BGSKeyword>(0x777775, "Update.esm");

  OCF_Glaive2H  = dataHandler->LookupForm<RE::BGSKeyword>(0x843, "OCF.esp");
  OCF_Pike2H    = dataHandler->LookupForm<RE::BGSKeyword>(0x847, "OCF.esp");
  OCF_Spear2H   = dataHandler->LookupForm<RE::BGSKeyword>(0x848, "OCF.esp");
  OCF_Trident2H = dataHandler->LookupForm<RE::BGSKeyword>(0x849, "OCF.esp");
  OCF_Halberd2H = dataHandler->LookupForm<RE::BGSKeyword>(0x844, "OCF.esp");
}

bool IsUnarmed(Type type)
{
  return type == Type::Unarm || type == Type::Werewolf || type == Type::Werebear ||
         type == Type::VampireLord;
}

Type GetActorEquipmentType(RE::Actor* actor, bool leftHand)
{
  if (!actor)
    return Type::None;

  auto equipment = actor->GetEquippedObject(leftHand);

  if (!equipment)
    return Type::None;

  if (equipment->IsArmor())
    return Type::Shield;

  if (equipment->formType == RE::FormType::Light)
    return Type::Torch;

  if (equipment->IsWeapon())
    return GetWeaponType(actor, equipment->As<RE::TESObjectWEAP>());

  logger::warn("Weapon::GetActorEquipmentType: Unsupported equipment type: {}",
               equipment->GetName());
  return Type::None;
}

Type GetWeaponType(RE::Actor* actor, RE::TESObjectWEAP* weapon)
{
  if (!weapon)
    return Type::None;

  const auto OCF_Fallback = [actor, weapon]() -> Type {
    switch (weapon->GetWeaponType()) {
    case RE::WEAPON_TYPE::kHandToHandMelee: {
      auto race = actor->GetRace();
      if (race == werewolfRace)
        return Type::Werewolf;
      else if (race == werebearRace)
        return Type::Werebear;
      else if (race == vampireLordRace)
        return Type::VampireLord;
      return Type::Unarm;
    }
    case RE::WEAPON_TYPE::kOneHandSword:
      return Type::Sword;
    case RE::WEAPON_TYPE::kOneHandDagger:
      return Type::Dagger;
    case RE::WEAPON_TYPE::kOneHandAxe:
      return Type::WarAxe;
    case RE::WEAPON_TYPE::kOneHandMace:
      return Type::Mace;
    case RE::WEAPON_TYPE::kTwoHandSword:
      return Type::GreatSword;
    case RE::WEAPON_TYPE::kTwoHandAxe:
      if (warhammerKeyword && weapon->HasKeyword(warhammerKeyword))
        return Type::WarHammer;
      return Type::BattleAxe;
    case RE::WEAPON_TYPE::kBow:
      return Type::Bow;
    case RE::WEAPON_TYPE::kStaff:
      return Type::Staff;
    case RE::WEAPON_TYPE::kCrossbow:
      return Type::Crossbow;
    default:
      logger::warn("Weapon::GetWeaponType: Why this thing gets here? {}", weapon->GetName());
      return DebugInfo(weapon, Type::None);
    }
  };

  // 完整类型探测依赖于OCF
  // 如果没有安装OCF
  // 使用原版判断
  if (!OCF_Installed)
    return OCF_Fallback();

  const auto DetectUnarmed = [actor, weapon]() -> Type {
    if (!weapon->HasKeyword(OCF_Unarmed))
      return DebugInfo(weapon, Type::None);

    auto race = actor->GetRace();
    if (race == werewolfRace)
      return Type::Werewolf;
    else if (race == werebearRace)
      return Type::Werebear;
    else if (race == vampireLordRace)
      return Type::VampireLord;
    return Type::Unarm;
  };

  const auto Detect1HType = [weapon]() -> Type {
    if (weapon->HasKeyword(OCF_SmallBladed1H)) {
      if (weapon->HasKeyword(OCF_Dagger1H))
        return Type::Dagger;
      if (weapon->HasKeyword(OCF_Claw1H))
        return Type::Claw;

      // 使用Dagger作为Fallbck
      return DebugInfo(weapon, Type::Dagger);
    } else if (weapon->HasKeyword(OCF_Bladed1H)) {
      if (weapon->HasKeyword(OCF_Sword1H))
        return Type::Sword;
      if (weapon->HasKeyword(OCF_Rapier1H))
        return Type::Rapier;
      if (weapon->HasKeyword(OCF_Katana1H))
        return Type::Katana;

      // 使用Sword作为Fallback
      return DebugInfo(weapon, Type::Sword);
    } else if (weapon->HasKeyword(OCF_Edged1H)) {
      // Axe和Hatchet都归为Axe类型
      return Type::WarAxe;

      if (weapon->HasKeyword(OCF_WarAxe1H))
        return Type::WarAxe;
      if (weapon->HasKeyword(OCF_Hatchet1H))
        return Type::WarAxe;

      // 使用WarAxe作为Fallback
      return DebugInfo(weapon, Type::WarAxe);
    } else if (weapon->HasKeyword(OCF_Blunt1H)) {
      if (weapon->HasKeyword(OCF_Mace1H))
        return Type::Mace;
      if (weapon->HasKeyword(OCF_Hammer1H))
        return Type::Mace;
      if (weapon->HasKeyword(OCF_Club1H))
        return Type::Mace;
      if (weapon->HasKeyword(OCF_Cestus1H))
        return Type::Cestus;
      if (weapon->HasKeyword(OCF_Whip1H))
        return Type::Whip;

      // 使用Mace作为Fallback
      return DebugInfo(weapon, Type::Mace);
    } else if (weapon->HasKeyword(OCF_Pole1H)) {
      // ShortPike和ShortSpear都归为ShortSpear类型
      return Type::ShortSpear;

      if (weapon->HasKeyword(OCF_Pike1H))
        return Type::ShortSpear;
      if (weapon->HasKeyword(OCF_Spear1H))
        return Type::ShortSpear;

      // 使用ShortSpear作为Fallback
      return DebugInfo(weapon, Type::ShortSpear);
    }
    return DebugInfo(weapon, Type::None);
  };

  const auto Detect2HType = [weapon]() -> Type {
    if (weapon->HasKeyword(OCF_Bladed2H)) {
      if (weapon->HasKeyword(OCF_Sword2H))
        return Type::GreatSword;
      if (weapon->HasKeyword(OCF_Katana2H))
        return Type::GreatKatana;

      // 使用GreatSword作为Fallback
      return DebugInfo(weapon, Type::GreatSword);
    } else if (weapon->HasKeyword(OCF_Edged2H)) {
      if (weapon->HasKeyword(OCF_BattleAxe2H))
        return Type::BattleAxe;

      // 使用BattleAxe作为Fallback
      return DebugInfo(weapon, Type::BattleAxe);
    } else if (weapon->HasKeyword(OCF_Blunt2H)) {
      if (weapon->HasKeyword(OCF_WarHammer2H))
        return Type::WarHammer;
      if (weapon->HasKeyword(OCF_Mace2H))
        return Type::WarHammer;
      if (weapon->HasKeyword(OCF_Club2H))
        return Type::WarHammer;
      if (weapon->HasKeyword(OCF_Quarterstaff2H))
        return Type::Quarterstaff;

      // 使用WarHammer作为Fallback
      return DebugInfo(weapon, Type::WarHammer);
    } else if (weapon->HasKeyword(OCF_Pole2H)) {
      if (weapon->HasKeyword(OCF_Glaive2H))
        return Type::Glaive;
      if (weapon->HasKeyword(OCF_Pike2H))
        return Type::Pike;
      if (weapon->HasKeyword(OCF_Spear2H))
        return Type::Pike;
      if (weapon->HasKeyword(OCF_Trident2H))
        return Type::Pike;
      if (weapon->HasKeyword(OCF_Halberd2H))
        return Type::Halberd;

      // 使用 Pike 作为 Fallback
      return DebugInfo(weapon, Type::Pike);
    }
    return DebugInfo(weapon, Type::None);
  };

  // 第一步确认单双手
  // 第二步根据OCF的关键词系统进行细分
  switch (weapon->GetWeaponType()) {
  case RE::WEAPON_TYPE::kHandToHandMelee:
    return DetectUnarmed();
  case RE::WEAPON_TYPE::kOneHandSword:
  case RE::WEAPON_TYPE::kOneHandDagger:
  case RE::WEAPON_TYPE::kOneHandAxe:
  case RE::WEAPON_TYPE::kOneHandMace:
    return Detect1HType();
  case RE::WEAPON_TYPE::kTwoHandSword:
  case RE::WEAPON_TYPE::kTwoHandAxe:
    return Detect2HType();
  case RE::WEAPON_TYPE::kBow:
    return Type::Bow;
  case RE::WEAPON_TYPE::kStaff:
    return Type::Staff;
  case RE::WEAPON_TYPE::kCrossbow:
    return Type::Crossbow;
  default:
    return DebugInfo(weapon, Type::None);
  }
}

Type GetBlockType(RE::Actor* actor)
{
  if (!actor)
    return Type::None;

  auto left  = GetActorEquipmentType(actor, true);
  auto right = GetActorEquipmentType(actor, false);

  // 盾牌和火把总是占据格挡动作，且只能装备在左手
  if (left == Type::Shield)
    return Type::Shield;
  else if (left == Type::Torch)
    return Type::Torch;

  // 左右手相同返回什么都行
  if (left == right)
    return left;

  // 如果左手是空手或特殊空手类型，返回右手的类型
  if (IsUnarmed(left))
    return right;

  // 如果右手是空手或特殊空手类型，返回左手的类型
  if (IsUnarmed(right))
    return left;

  // 其他类型的格挡动作都用右手武器的类型来判断
  return right;
}

float GetBasePostureDamage(Type type)
{
  auto id = static_cast<EnumType>(type);
  if (Settings::basePostureDamageMap.contains(id))
    return Settings::basePostureDamageMap[id];
  logger::warn("Weapon::GetBasePostureDamage: Unsupported weapon type: {}",
               magic_enum::enum_name(type));
  return 0.0f;
}

float GetBaseStaminaConsumption(Type type)
{
  auto id = static_cast<EnumType>(type);
  if (Settings::baseStaminaCostMap.contains(id))
    return Settings::baseStaminaCostMap[id];
  logger::warn("Weapon::GetBaseStaminaConsumption: Unsupported weapon type: {}",
               magic_enum::enum_name(type));
  return 0.0f;
}

float GetBlockStrength(Type type)
{
  auto id = static_cast<EnumType>(type);
  if (Settings::blockStrengthMap.contains(id))
    return Settings::blockStrengthMap[id];
  logger::warn("Weapon::GetBlockStrength: Unsupported block type: {}", magic_enum::enum_name(type));
  return 1.0f;
}

float GetBaseExecutionMultiplier(RE::Actor* actor)
{
  auto type = GetActorEquipmentType(actor, false);
  auto id   = static_cast<EnumType>(type);
  if (Settings::executionDamageMultMap.contains(id))
    return Settings::executionDamageMultMap[id];
  logger::warn("Weapon::GetBaseExecutionMultiplier: Unsupported weapon type: {}",
               magic_enum::enum_name(type));
  return 1.0f;
}
}  // namespace Weapon