#include "Combat/Weapon.h"

#include "Core/Settings.h"

namespace Weapon
{
// 原版
static RE::BGSKeyword* warhammerKeyword = nullptr;
static RE::TESRace* werewolfRace        = nullptr;
static RE::TESRace* vampireLordRace     = nullptr;

// 动画军械库
static bool animatedArmoryLoaded = false;

// 重型军械库
static bool heavyArmoryLoaded = false;

void Initialize()
{
  warhammerKeyword = RE::TESForm::LookupByID<RE::BGSKeyword>(0x6D930);
}

Type GetWeaponType(RE::TESObjectWEAP* weapon)
{
  if (!weapon)
    return Type::Unarm;

  switch (weapon->GetWeaponType()) {
  case RE::WEAPON_TYPE::kHandToHandMelee:
    return Type::Fist;
  case RE::WEAPON_TYPE::kOneHandSword:
    return Type::Sword;
  case RE::WEAPON_TYPE::kOneHandDagger:
    return Type::Dagger;
  case RE::WEAPON_TYPE::kOneHandAxe:
    return Type::Axe;
  case RE::WEAPON_TYPE::kOneHandMace:
    return Type::Mace;
  case RE::WEAPON_TYPE::kTwoHandSword:
    return Type::GreatSword;
  case RE::WEAPON_TYPE::kTwoHandAxe: {
    if (warhammerKeyword && weapon->HasKeyword(warhammerKeyword))
      return Type::GreatMace;
    return Type::GreatAxe;
  }
  case RE::WEAPON_TYPE::kBow:
    return Type::Bow;
  case RE::WEAPON_TYPE::kStaff:
    return Type::Staff;
  case RE::WEAPON_TYPE::kCrossbow:
    return Type::Crossbow;
  default:
    logger::warn("Weapon::GetWeaponType: Why this thing gets here? {}", weapon->GetFullName());
    return Type::Claw;
  }
}

Type GetActorEquipmentType(RE::Actor* actor, bool leftHand)
{
  if (!actor)
    return Type::None;

  auto equipment = actor->GetEquippedObject(leftHand);

  if (!equipment)
    return Type::Unarm;

  if (equipment->IsArmor())
    return Type::Shield;

  if (equipment->formType == RE::FormType::Light)
    return Type::Torch;

  if (equipment->IsWeapon())
    return GetWeaponType(equipment->As<RE::TESObjectWEAP>());

  logger::warn("Weapon::GetActorEquipmentType: Unsupported equipment type: {}",
               equipment->GetName());
  return Type::None;
}

float GetBasePostureDamage(Type type)
{
  switch (type) {
  case Type::Unarm:
    return Settings::fNormalAttackPostureDamage_Unarm;
  case Type::Dagger:
    return Settings::fNormalAttackPostureDamage_Dagger;
  case Type::Sword:
    return Settings::fNormalAttackPostureDamage_Sword;
  case Type::Axe:
    return Settings::fNormalAttackPostureDamage_Axe;
  case Type::Mace:
    return Settings::fNormalAttackPostureDamage_Mace;
  case Type::GreatSword:
    return Settings::fNormalAttackPostureDamage_GreatSword;
  case Type::GreatAxe:
    return Settings::fNormalAttackPostureDamage_GreatAxe;
  case Type::GreatMace:
    return Settings::fNormalAttackPostureDamage_GreatMace;
  case Type::Shield:
    return Settings::fBashPostureDamage_Shield;
  default:
    logger::warn("Weapon::GetBasePostureDamage: Unsupported weapon type: {}",
                 static_cast<int>(type));
    return Settings::fNormalAttackPostureDamage_Unarm;
  }
}

float GetBaseStaminaConsumption(Type type)
{
  switch (type) {
  case Type::Unarm:
    return Settings::fNormalAttackStaminaCostBase_Unarm;
  case Type::Dagger:
    return Settings::fNormalAttackStaminaCostBase_Dagger;
  case Type::Sword:
    return Settings::fNormalAttackStaminaCostBase_Sword;
  case Type::Axe:
    return Settings::fNormalAttackStaminaCostBase_Axe;
  case Type::Mace:
    return Settings::fNormalAttackStaminaCostBase_Mace;
  case Type::GreatSword:
    return Settings::fNormalAttackStaminaCostBase_GreatSword;
  case Type::GreatAxe:
    return Settings::fNormalAttackStaminaCostBase_GreatAxe;
  case Type::GreatMace:
    return Settings::fNormalAttackStaminaCostBase_GreatMace;
  case Type::Fist:
    return Settings::fNormalAttackStaminaCostBase_Fist;
  default:
    logger::warn("Weapon::GetBaseStaminaConsumption: Unsupported weapon type: {}",
                 static_cast<int>(type));
    return Settings::fNormalAttackStaminaCostBase_Unarm;
  }
}
}  // namespace Weapon