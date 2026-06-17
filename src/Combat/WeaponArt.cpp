#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"
#include "Utils.h"

#include "Combat/Damage.h"
#include "Combat/Poise.h"
#include "Combat/Posture.h"
#include "Combat/Stagger.h"
#include "Combat/Stamina.h"
#include "Data/Race.h"
#include "GUI/UI.h"

#include "magic_enum/magic_enum_flags.hpp"
#include "nlohmann/json.hpp"

namespace WeaponArt
{

using AvailableWeaponType = WeaponArtInfo::AvailableWeaponType;
using AvailableWeapon     = WeaponArtInfo::AvailableWeapon;

using StageData  = WeaponArtInfo::StageData;
using AttackData = WeaponArtInfo::AttackData;

using Skill     = WeaponArtInfo::Skill;
using SpellInfo = WeaponArtInfo::SpellInfo;

constexpr inline AvailableWeapon operator|(AvailableWeapon lhs, AvailableWeapon rhs)
{
  auto l = static_cast<AvailableWeaponType>(lhs);
  auto r = static_cast<AvailableWeaponType>(rhs);
  return static_cast<AvailableWeapon>(l | r);
}

constexpr inline AvailableWeapon operator&(AvailableWeapon lhs, AvailableWeapon rhs)
{
  auto l = static_cast<AvailableWeaponType>(lhs);
  auto r = static_cast<AvailableWeaponType>(rhs);
  return static_cast<AvailableWeapon>(l & r);
}

constexpr inline bool operator>=(AvailableWeapon lhs, AvailableWeapon rhs)
{
  auto l = static_cast<AvailableWeaponType>(lhs);
  auto r = static_cast<AvailableWeaponType>(rhs);
  return (l | r) == l;
}

constexpr AvailableWeapon weightMask =
    AvailableWeapon::Light | AvailableWeapon::Normal | AvailableWeapon::Heavy;

constexpr AvailableWeapon attackMask = AvailableWeapon::Slash | AvailableWeapon::Thrust |
                                       AvailableWeapon::Blunt | AvailableWeapon::Polearm |
                                       AvailableWeapon::Range;

constexpr AvailableWeapon familyMask = AvailableWeapon::Sword | AvailableWeapon::Axe |
                                       AvailableWeapon::Hammer | AvailableWeapon::Fist |
                                       AvailableWeapon::Katana | AvailableWeapon::Spear |
                                       AvailableWeapon::Stick | AvailableWeapon::Whip |
                                       AvailableWeapon::Bow | AvailableWeapon::Crossbow;

// 技能等级
const static std::unordered_map<Skill, RE::ActorValue> skillMap{
    {Skill::OneHanded, RE::ActorValue::kOneHanded},
    {Skill::TwoHanded, RE::ActorValue::kTwoHanded},
    {Skill::Archery, RE::ActorValue::kArchery},
    {Skill::Block, RE::ActorValue::kBlock},
    {Skill::Smithing, RE::ActorValue::kSmithing},
    {Skill::HeavyArmor, RE::ActorValue::kHeavyArmor},
    {Skill::LightArmor, RE::ActorValue::kLightArmor},
    {Skill::Pickpocket, RE::ActorValue::kPickpocket},
    {Skill::Lockpicking, RE::ActorValue::kLockpicking},
    {Skill::Sneak, RE::ActorValue::kSneak},
    {Skill::Alchemy, RE::ActorValue::kAlchemy},
    {Skill::Speech, RE::ActorValue::kSpeech},
    {Skill::Alteration, RE::ActorValue::kAlteration},
    {Skill::Conjuration, RE::ActorValue::kConjuration},
    {Skill::Destruction, RE::ActorValue::kDestruction},
    {Skill::Illusion, RE::ActorValue::kIllusion},
    {Skill::Restoration, RE::ActorValue::kRestoration},
    {Skill::Enchanting, RE::ActorValue::kEnchanting}};

// 常驻技能百分比变化，数值x，表示x%
const static std::unordered_map<Skill, RE::ActorValue> skillModMap{
    {Skill::OneHanded, RE::ActorValue::kOneHandedModifier},
    {Skill::TwoHanded, RE::ActorValue::kTwoHandedModifier},
    {Skill::Archery, RE::ActorValue::kMarksmanModifier},
    {Skill::Block, RE::ActorValue::kBlockModifier},
    {Skill::Smithing, RE::ActorValue::kSmithingModifier},
    {Skill::HeavyArmor, RE::ActorValue::kHeavyArmorModifier},
    {Skill::LightArmor, RE::ActorValue::kLightArmorModifier},
    {Skill::Pickpocket, RE::ActorValue::kPickpocketModifier},
    {Skill::Lockpicking, RE::ActorValue::kLockpickingModifier},
    {Skill::Sneak, RE::ActorValue::kSneakingModifier},
    {Skill::Alchemy, RE::ActorValue::kAlchemyModifier},
    {Skill::Speech, RE::ActorValue::kSpeechcraftModifier},
    {Skill::Alteration, RE::ActorValue::kAlterationModifier},
    {Skill::Conjuration, RE::ActorValue::kConjurationModifier},
    {Skill::Destruction, RE::ActorValue::kDestructionModifier},
    {Skill::Illusion, RE::ActorValue::kIllusionModifier},
    {Skill::Restoration, RE::ActorValue::kRestorationModifier},
    {Skill::Enchanting, RE::ActorValue::kEnchantingModifier}};

// 临时技能百分比变化，数值x，表示x%
const static std::unordered_map<Skill, RE::ActorValue> skillPowerMap{
    {Skill::OneHanded, RE::ActorValue::kOneHandedPowerModifier},
    {Skill::TwoHanded, RE::ActorValue::kTwoHandedPowerModifier},
    {Skill::Archery, RE::ActorValue::kMarksmanPowerModifier},
    {Skill::Block, RE::ActorValue::kBlockPowerModifier},
    {Skill::Smithing, RE::ActorValue::kSmithingPowerModifier},
    {Skill::HeavyArmor, RE::ActorValue::kHeavyArmorPowerModifier},
    {Skill::LightArmor, RE::ActorValue::kLightArmorPowerModifier},
    {Skill::Pickpocket, RE::ActorValue::kPickpocketPowerModifier},
    {Skill::Lockpicking, RE::ActorValue::kLockpickingPowerModifier},
    {Skill::Sneak, RE::ActorValue::kSneakingPowerModifier},
    {Skill::Alchemy, RE::ActorValue::kAlchemyPowerModifier},
    {Skill::Speech, RE::ActorValue::kSpeechcraftPowerModifier},
    {Skill::Alteration, RE::ActorValue::kAlterationPowerModifier},
    {Skill::Conjuration, RE::ActorValue::kConjurationPowerModifier},
    {Skill::Destruction, RE::ActorValue::kDestructionPowerModifier},
    {Skill::Illusion, RE::ActorValue::kIllusionPowerModifier},
    {Skill::Restoration, RE::ActorValue::kRestorationPowerModifier},
    {Skill::Enchanting, RE::ActorValue::kEnchantingPowerModifier}};

// 获取技能等级和总倍率
std::tuple<float, float> GetSkillData(RE::Actor* actor, Skill skill)
{
  float skillLevel         = 0.0f;
  float SkillModifier      = 0.0f;
  float skillPowerModifier = 0.0f;

  if (auto it = skillMap.find(skill); it != skillMap.end())
    skillLevel = actor->AsActorValueOwner()->GetActorValue(it->second);
  if (auto it = skillModMap.find(skill); it != skillModMap.end())
    SkillModifier = actor->AsActorValueOwner()->GetActorValue(it->second);
  if (auto it = skillPowerMap.find(skill); it != skillPowerMap.end())
    skillPowerModifier = actor->AsActorValueOwner()->GetActorValue(it->second);
  return {skillLevel, 1 + (SkillModifier + skillPowerModifier) / 100.0f};
};

AvailableWeapon WeaponTypeMapping(Weapon::Type type)
{
  using AW = AvailableWeapon;

  switch (type) {
  // ── 不参与通用战技 ──
  case Weapon::Type::None:
  case Weapon::Type::Unarm:
    return AW::None;

  // ── 单手小型刃器 ──
  case Weapon::Type::Dagger:
    return AW::Light | AW::Slash | AW::Thrust | AW::Sword;
  case Weapon::Type::Claw:
    return AW::Light | AW::Slash | AW::Fist;

  // ── 单手剑 / 刀 / 斧 / 锤 / 鞭 ──
  case Weapon::Type::Sword:
    return AW::Normal | AW::Slash | AW::Thrust | AW::Sword;
  case Weapon::Type::Rapier:
    return AW::Normal | AW::Thrust | AW::Sword;
  case Weapon::Type::Katana:
    return AW::Normal | AW::Slash | AW::Thrust | AW::Katana;
  case Weapon::Type::WarAxe:
    return AW::Normal | AW::Slash | AW::Axe;
  case Weapon::Type::Mace:
    return AW::Normal | AW::Blunt | AW::Hammer;
  case Weapon::Type::Cestus:
    return AW::Light | AW::Blunt | AW::Fist;
  case Weapon::Type::Whip:
    return AW::Light | AW::Blunt | AW::Whip;

  // ── 单手长柄 ──
  case Weapon::Type::ShortSpear:
    return AW::Normal | AW::Thrust | AW::Polearm | AW::Spear;

  // ── 双手剑 / 刀 / 斧 / 锤 ──
  case Weapon::Type::GreatSword:
    return AW::Heavy | AW::Slash | AW::Thrust | AW::Sword;
  case Weapon::Type::GreatKatana:
    return AW::Heavy | AW::Slash | AW::Thrust | AW::Katana;
  case Weapon::Type::BattleAxe:
    return AW::Heavy | AW::Slash | AW::Axe;
  case Weapon::Type::WarHammer:
    return AW::Heavy | AW::Blunt | AW::Hammer;

  // ── 双手长柄 ──
  case Weapon::Type::Glaive:
    return AW::Heavy | AW::Slash | AW::Polearm | AW::Spear;
  case Weapon::Type::Pike:
    return AW::Heavy | AW::Thrust | AW::Polearm | AW::Spear;
  case Weapon::Type::Halberd:
    return AW::Heavy | AW::Slash | AW::Thrust | AW::Polearm | AW::Spear;

  // ── 棍棒 ──
  case Weapon::Type::Quarterstaff:
    // 长棍虽是钝器，但双手使用且攻击动作类似长柄
    return AW::Heavy | AW::Blunt | AW::Polearm | AW::Stick;
  case Weapon::Type::Staff:
    // 法杖不具有攻击动作，但这里先分类
    return AW::Normal | AW::Blunt | AW::Polearm | AW::Stick;

  // ── 远程 ──
  case Weapon::Type::Bow:
    return AW::Range | AW::Bow;
  case Weapon::Type::Crossbow:
    return AW::Range | AW::Crossbow;

  // ── 特殊装备 ──
  case Weapon::Type::Shield:
  case Weapon::Type::Torch:
    return AW::None;

  default:
    return AW::None;
  }
}

WeaponArtInfo::WeaponArtInfo(std::int32_t id, const std::string& name,
                             const std::string& description, AvailableWeapon availableWeapon,
                             const std::vector<RE::FormID>& weapons,
                             const std::unordered_map<std::uint8_t, StageData>& stages,
                             const std::unordered_map<std::uint32_t, SpellInfo>& spells,
                             std::uint8_t consumePoint, std::uint8_t unlockLevel, bool ownAtStart,
                             bool needPrepare)
{
  this->id              = id;
  this->name            = std::move(name);
  this->description     = std::move(description);
  this->availableWeapon = availableWeapon;
  this->weapons         = std::move(weapons);
  this->stages          = std::move(stages);
  this->spells          = std::move(spells);
  this->consumePoint    = consumePoint;
  this->unlockLevel     = unlockLevel;
  this->ownAtStart      = ownAtStart;
  this->needPrepare     = needPrepare;
}

bool WeaponArtInfo::IsWeaponAllowed(RE::TESObjectWEAP* weapon) const
{
  if (!weapon)
    return false;

  // 优先检查Unique条件
  if (availableWeapon == AvailableWeapon::Unique)
    return std::find(weapons.begin(), weapons.end(), weapon->GetFormID()) != weapons.end();

  // 武器类型
  auto weaponType = WeaponTypeMapping(Weapon::GetWeaponType(weapon));

  auto availableAttack = availableWeapon & attackMask;

  // 如果没有指定攻击类型要求，则视为通配
  if (availableAttack == AvailableWeapon::None)
    return (availableWeapon | attackMask) >= weaponType;

  // 如果指定了攻击类型要求，则武器的攻击类型必须满足要求
  auto weaponAttack = weaponType & attackMask;
  return (availableWeapon | attackMask) >= (weaponType | attackMask) &&
         weaponAttack >= availableAttack;
}

const std::optional<StageData> WeaponArtInfo::GetStageData(std::uint8_t stageID) const
{
  if (auto it = stages.find(stageID); it != stages.end())
    return it->second;
  return std::nullopt;
}

const std::optional<AttackData> WeaponArtInfo::GetAttackData(std::uint8_t stageID,
                                                             std::uint8_t attackID) const
{
  auto it = stages.find(stageID);
  if (it != stages.end()) {
    const auto& stageData = it->second;
    auto it2              = stageData.attacks.find(attackID);
    if (it2 != stageData.attacks.end())
      return it2->second;
  }
  return std::nullopt;
}

const std::optional<SpellInfo> WeaponArtInfo::GetSpellInfo(std::uint32_t hash) const
{
  if (auto it = spells.find(hash); it != spells.end())
    return it->second;
  return std::nullopt;
}

PlayerStat::PlayerStat()
{
  // PlayerStat在Manager之后初始化，因此可以安全地访问Manager中的数据

  Serialization::RegisterSaveCallback(serialType, [](SKSE::SerializationInterface* serial) {
    serial->WriteRecordData(&exp, sizeof(exp));
    serial->WriteRecordData(&level, sizeof(level));
    serial->WriteRecordData(&point, sizeof(point));

    // 保存已拥有的战技ID列表
    auto count = static_cast<std::uint32_t>(ownedArts.size());
    serial->WriteRecordData(&count, sizeof(count));
    for (const auto& artID : ownedArts) {
      // 保存时无需验证artID的有效性，因为读取必定优先验证
      serial->WriteRecordData(&artID, sizeof(artID));
    }

    // 保存已解锁的战技ID列表
    count = static_cast<std::uint32_t>(unlockedArts.size());
    serial->WriteRecordData(&count, sizeof(count));
    for (const auto& artID : unlockedArts) {
      // 保存时无需验证artID的有效性，因为读取必定优先验证
      serial->WriteRecordData(&artID, sizeof(artID));
    }
  });

  Serialization::RegisterLoadCallback(serialType, [](SKSE::SerializationInterface* serial) {
    serial->ReadRecordData(&exp, sizeof(exp));
    serial->ReadRecordData(&level, sizeof(level));
    serial->ReadRecordData(&point, sizeof(point));

    // 插入所有一开始就拥有的战技ID
    auto arts = Manager::GetAllWeaponArts();
    for (const auto& art : arts) {
      if (art->OwnAtStart())
        ownedArts.insert(art->GetID());
    }

    std::uint32_t count = 0;

    // 读取已拥有的战技ID列表，并验证每个ID的有效性，确保只加载已定义的战技ID
    serial->ReadRecordData(&count, sizeof(count));
    for (std::uint32_t i = 0; i < count; ++i) {
      std::int32_t artID = 0;
      serial->ReadRecordData(&artID, sizeof(artID));
      // 验证artID的有效性，确保只加载已定义的战技ID
      if (Manager::IsValidWeaponArtID(artID))
        ownedArts.insert(artID);
    }

    // 读取已解锁的战技ID列表，并验证每个ID的有效性，确保只加载已定义的战技ID
    serial->ReadRecordData(&count, sizeof(count));
    for (std::uint32_t i = 0; i < count; ++i) {
      std::int32_t artID = 0;
      serial->ReadRecordData(&artID, sizeof(artID));
      // 验证artID的有效性，确保只加载已定义的战技ID
      if (Manager::IsValidWeaponArtID(artID))
        unlockedArts.insert(artID);
    }
  });

  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    exp   = 0.0f;
    level = 1;
    point = 3;
    ownedArts.clear();
    unlockedArts.clear();
  });
}

bool PlayerStat::IsOwned(std::int32_t artID)
{
  return ownedArts.find(artID) != ownedArts.end();
}

bool PlayerStat::IsUnlocked(std::int32_t artID)
{
  return unlockedArts.find(artID) != unlockedArts.end();
}

void PlayerStat::AddExp(float value)
{
  if (value <= 0.0f)
    return;

  exp += value;

  // 升级所需经验公式：100 * 当前等级
  // 限制等级上限为100级，防止经验要求太高
  float requiredExp = 100.0f * level;
  while (exp >= requiredExp) {
    exp -= requiredExp;
    level++;
    point += 1;  // 每升一级获得1点战技点数
    requiredExp = 100.0f * level;
    RE::SendHUDMessage::ShowHUDMessage(
        std::format("WeaponArt Level Up, Current Level: {}!", level).data());
    // 抵达上限后等级不会增加，但仍然可以获得经验和战技点数
    if (level >= 100)
      level = 100;
  }
}

bool PlayerStat::SetOwned(std::int32_t artID)
{
  if (ownedArts.find(artID) != ownedArts.end())
    return true;  // 已拥有

  auto* art = Manager::GetWeaponArtInfo(artID);
  if (!art)  // 无效的战技ID
    return false;
  ownedArts.insert(artID);
  return true;
}

bool PlayerStat::UnlockArt(std::int32_t artID)
{
  if (ownedArts.find(artID) == ownedArts.end())
    return false;  // 不拥有该战技，无法解锁

  if (unlockedArts.find(artID) != unlockedArts.end())
    return true;  // 已解锁

  auto* art = Manager::GetWeaponArtInfo(artID);
  if (!art)  // 无效的战技ID
    return false;
  if (art->GetUnlockLevel() > level)  // 等级不足
    return false;
  if (art->GetConsumePoint() > point)  // 战技点数不足
    return false;
  point -= art->GetConsumePoint();
  unlockedArts.insert(art->GetID());
  return true;
}

Manager::Manager()
{

  auto dataHandle = RE::TESDataHandler::GetSingleton();
  if (!dataHandle) {
    logger::error("Failed to get TESDataHandler singleton.");
    return;
  }

  // 从JSON文件加载战技信息
  const std::string weaponArtDir = std::string(Settings::SettingsDir) + "WeaponArt/";
  for (const auto& entry : std::filesystem::directory_iterator(weaponArtDir)) {
    if (!entry.is_regular_file() || entry.path().extension() != ".json")
      continue;

    try {
      std::ifstream file(entry.path());
      nlohmann::json j;
      file >> j;

      for (const auto& [key, value] : j.items()) {
        std::int32_t id = Utils::hash(key);
        // 0为无效ID，跳过
        if (id == 0) {
          logger::warn("Invalid Weapon Art ID for {} in file {}. Skipping.", key,
                       entry.path().string());
          continue;
        }
        // 一般hash值不会发生碰撞，但为了安全起见，仍然验证ID的唯一性
        if (artMap.find(id) != artMap.end()) {
          logger::warn("Hash collision for Weapon Art {} in file {}. Skipping.", key,
                       entry.path().string());
          continue;
        }

        // 名字必须存在
        std::string name = value.value("name", "");
        if (name.empty()) {
          logger::warn("Missing name for Weapon Art {} in file {}. Skipping.", key,
                       entry.path().string());
          continue;
        }

        // 描述可以不存在，默认为空字符串
        std::string description = value.value("description", "");

        // 武器可以不存在，默认为空列表
        std::vector<std::string> weaponStrs = value.value("weapons", std::vector<std::string>{});

        // 可用武器类型可以不存在，默认为None
        std::vector<std::string> availableWeaponStrs =
            value.value("availableWeapon", std::vector<std::string>{});

        // 不必对weaponStrs判空
        std::vector<RE::FormID> weapons{};
        for (const auto& w : weaponStrs) {
          auto split = Utils::split(w, '|');
          if (split.size() != 2) {
            logger::warn("Invalid weapon '{}' for Weapon Art {} in file {}. Skipping this weapon.",
                         w, key, entry.path().string());
            continue;
          }
          auto res = dataHandle->LookupFormID(Utils::hash(split[0]), split[1]);
          if (res)
            weapons.push_back(res);
        }

        AvailableWeapon availableWeapon = AvailableWeapon::None;
        for (const auto& awStr : availableWeaponStrs) {
          if (auto awOpt = magic_enum::enum_flags_cast<AvailableWeapon>(awStr); awOpt.has_value()) {
            availableWeapon = availableWeapon | awOpt.value();
          } else {
            logger::warn(
                "Invalid weapon art type '{}' for Weapon Art {} in file {}. Skipping this type.",
                awStr, key, entry.path().string());
          }
        }

        // 不必判断None，会默认通配所有类型
        // 如果不是Unique类型，则需要后处理保证匹配
        if (availableWeapon != AvailableWeapon::Unique) {
          // 如果没有指定重量要求，则视为通配
          if ((availableWeapon & weightMask) == AvailableWeapon::None)
            availableWeapon = availableWeapon | weightMask;
          // 如果没有指定武器家族要求，则视为通配
          if ((availableWeapon & familyMask) == AvailableWeapon::None)
            availableWeapon = availableWeapon | familyMask;
        }

        // 攻击数据必须存在
        // 任何数据非法都直接中断解析并清空stages
        bool stageSuccess = true;
        std::unordered_map<std::uint8_t, StageData> stages{};
        auto stagesJson = value.value("stages", nlohmann::json::object());
        if (stagesJson.is_object()) {
          for (const auto& [stageKey, stageDataJson] : stagesJson.items()) {
            StageData stageData;
            auto stageID = Utils::toInt(stageKey);

            if (!stageID) {
              logger::warn("Invalid stage ID {} in Weapon Art {} in file {}. Clean and Skip.",
                           stageKey, key, entry.path().string());
              stageSuccess = false;
              break;
            }

            stageData.manaCost = stageDataJson.value("manaCost", NaN);
            stageData.minMana  = stageDataJson.value("minMana", NaN);

            if (std::isnan(stageData.manaCost) || std::isnan(stageData.minMana)) {
              logger::warn(
                  "Invalid Stage Data at Stage {} in Weapon Art {} in file {}. Clean and Skip.",
                  stageKey, key, entry.path().string());
              stageSuccess = false;
              break;
            }

            bool attackSuccess = true;
            std::unordered_map<std::uint8_t, AttackData> attacks;
            auto attacksJson = stageDataJson.value("attacks", nlohmann::json::object());
            if (attacksJson.is_object()) {
              for (const auto& [attackKey, attackDataJson] : attacksJson.items()) {
                AttackData attackData;
                auto attackID = Utils::toInt(attackKey);
                if (!attackID) {
                  logger::warn("Invalid attack ID {} at Stage {} in Weapon Art {} in file {}. "
                               "Clean and Skip.",
                               attackKey, stageKey, key, entry.path().string());
                  attackSuccess = false;
                  break;
                }

                attackData.left        = attackDataJson.value("left", false);
                attackData.right       = attackDataJson.value("right", false);
                attackData.powerAttack = attackDataJson.value("powerAttack", false);

                attackData.staminaMult       = attackDataJson.value("staminaMult", NaN);
                attackData.damageMult        = attackDataJson.value("damageMult", NaN);
                attackData.poiseDamageMult   = attackDataJson.value("poiseDamageMult", NaN);
                attackData.postureDamageMult = attackDataJson.value("postureDamageMult", NaN);

                attackData.subStaminaMult       = attackDataJson.value("subStaminaMult", NaN);
                attackData.subDamageMult        = attackDataJson.value("subDamageMult", NaN);
                attackData.subPoiseDamageMult   = attackDataJson.value("subPoiseDamageMult", NaN);
                attackData.subPostureDamageMult = attackDataJson.value("subPostureDamageMult", NaN);

                if (std::isnan(attackData.staminaMult) || std::isnan(attackData.damageMult) ||
                    std::isnan(attackData.poiseDamageMult) ||
                    std::isnan(attackData.postureDamageMult) ||
                    std::isnan(attackData.subStaminaMult) || std::isnan(attackData.subDamageMult) ||
                    std::isnan(attackData.subPoiseDamageMult) ||
                    std::isnan(attackData.subPostureDamageMult)) {
                  logger::warn("Invalid attack Data at Attack {} at Stage {} in Weapon Art {} in "
                               "file {}. Clean and Skip.",
                               attackKey, stageKey, key, entry.path().string());
                  attackSuccess = false;
                  break;
                }

                attacks.emplace(attackID.value(), attackData);
              }
            }

            if (attackSuccess) {
              stageData.attacks = std::move(attacks);
              stages.emplace(stageID.value(), std::move(stageData));
            } else {
              logger::warn("Failed to Parse AttackData at Stage {} in Weapon Art {} in file {}. "
                           "Clean and Skip.",
                           stageKey, key, entry.path().string());
              stageSuccess = false;
              break;
            }
          }
        }
        if (stages.empty()) {
          logger::warn("Empty StageData for Weapon Art {} in file {}. Skipping", key,
                       entry.path().string());
          continue;
        }
        if (!stageSuccess) {
          logger::warn("Failed to Parse StageData in Weapon Art {} in file {}. Clean and Skip.",
                       key, entry.path().string());
          stages.clear();
          continue;
        }

        // 法术特效可以不存在，默认为空对象
        bool spellSuccess = true;
        auto spellsJson   = value.value("spells", nlohmann::json::object());
        std::unordered_map<std::uint32_t, SpellInfo> spells{};
        if (spellsJson.is_object()) {
          for (const auto& [spellKey, spellValue] : spellsJson.items()) {
            // 因为event处理中会把所以字符转为小写，所以这里也统一转为小写来hash
            std::string lower = spellKey;
            std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
            std::uint32_t spellHash = Utils::hash(lower);

            std::string modName   = spellValue.value("mod", "");
            std::string formIDStr = spellValue.value("formID", "");
            auto formID           = Utils::toInt(formIDStr, 16);
            if (!formID) {
              logger::warn("Invalid form ID '{}' for Weapon Art {} in file {}. Clean and Skip.",
                           formIDStr, key, entry.path().string());
              spellSuccess = false;
              break;
            }

            auto spellFormID = dataHandle->LookupFormID(formID.value(), modName);
            if (!spellFormID) {
              logger::warn(
                  "Invalid spell reference '{}' for Weapon Art {} in file {}. Clean and Skip.",
                  spellKey, key, entry.path().string());
              spellSuccess = false;
              break;
            }
            auto* spell = RE::TESForm::LookupByID<RE::SpellItem>(spellFormID);
            if (!spell || spell->formType != RE::FormType::Spell) {
              logger::warn("Form ID '{}' does not correspond to a valid spell for Weapon Art {} "
                           "in file {}. Clean and Skip.",
                           formID.value(), key, entry.path().string());
              spellSuccess = false;
              break;
            }

            bool selfCast        = spellValue.value("selfCast", false);
            float effectiveness  = spellValue.value("effectiveness", 1.0f);
            float stdMagnitude   = spellValue.value("stdMagnitude", 0.0f);
            std::string skillStr = spellValue.value("skill", "None");
            Skill skill          = magic_enum::enum_cast<Skill>(skillStr).value_or(Skill::None);
            if (skill == Skill::None) {
              logger::warn(
                  "Invalid skill '{}' for spell '{}' in Weapon Art {} in file {}. Clean and Skip.",
                  skillStr, spellKey, key, entry.path().string());
              spellSuccess = false;
              break;
            }

            float factor = spellValue.value("factor", 1.0f);
            if (factor < 0.0f) {
              logger::warn(
                  "Invalid factor {} for spell '{}' in Weapon Art {} in file {}. Clean and Skip.",
                  factor, spellKey, key, entry.path().string());
              spellSuccess = false;
              break;
            }

            spells[spellHash] =
                SpellInfo{spell, effectiveness, stdMagnitude, factor, skill, selfCast};
          }
        }
        if (!spellSuccess) {
          logger::warn("Failed to Parse SpellInfo in Weapon Art {} in file {}. Clean and Skip.",
                       key, entry.path().string());
          spells.clear();
          continue;
        }

        // 消耗的战技点数和解锁所需等级，默认为0和1
        std::uint8_t consumePoint = value.value("consumePoint", 0);
        std::uint8_t unlockLevel  = value.value("unlockLevel", 1);

        // 是否一开始就拥有该战技，默认为false
        bool ownAtStart = value.value("ownAtStart", false);

        // 是否准备动画，不要求一定存在，默认为false
        bool needPrepare = value.value("needPrepare", false);

        // 如果想要查看ID，可以在对应的战技定义中添加"verbose": true字段
        bool verbose = value.value("verbose", false);
        // if (verbose)
        logger::info("Loaded Weapon Art {} (ID: {}) from file {}.", name, id,
                     entry.path().string());

        WeaponArtInfo art(id, name, description, availableWeapon, std::move(weapons),
                          std::move(stages), std::move(spells), consumePoint, unlockLevel,
                          ownAtStart, needPrepare);
        artMap[id] = std::move(art);
      }
    } catch (const std::exception& e) {
      logger::error("Failed to load Weapon Art from file {}: {}", entry.path().string(), e.what());
    }
  }

  // 序列化

  // 保存时直接转为持久FormID，不用考虑合法性问题，因为读取时会验证
  Serialization::RegisterSaveCallback(serialType, [](SKSE::SerializationInterface* serial) {
    std::unordered_map<std::uint64_t, std::int32_t> persistMap;
    {
      std::lock_guard<std::mutex> lock(mtx_infoMap);
      for (const auto& [formID, artID] : infoMap) {
        auto persist = Serialization::ToPersistForm(formID);
        if (persist && artMap.contains(artID))
          persistMap[persist] = artID;
      }
    }

    auto size = static_cast<std::uint32_t>(persistMap.size());
    serial->WriteRecordData(&size, sizeof(size));
    for (const auto& [persistID, id] : persistMap) {
      serial->WriteRecordData(&persistID, sizeof(persistID));
      serial->WriteRecordData(&id, sizeof(id));
    }
  });

  // 读取时保证FormID和战技的ID的有效性，避免加载到非法数据
  Serialization::RegisterLoadCallback(serialType, [](SKSE::SerializationInterface* serial) {
    std::unordered_map<RE::FormID, std::int32_t> tempMap;
    std::uint32_t size;
    if (serial->ReadRecordData(&size, sizeof(size))) {
      for (std::uint32_t i = 0; i < size; ++i) {
        std::uint64_t persistID;
        std::int32_t artID;
        if (serial->ReadRecordData(&persistID, sizeof(persistID)) &&
            serial->ReadRecordData(&artID, sizeof(artID))) {
          auto formID = Serialization::ToForm(persistID);
          if (formID)
            tempMap[formID] = artID;
        }
      }
    }

    {
      std::lock_guard<std::mutex> lock(mtx_infoMap);
      infoMap.clear();
      for (const auto& [formID, artID] : tempMap) {
        if (artMap.contains(artID))
          infoMap[formID] = artID;
      }
    }
  });

  // 重置时直接清空数据
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    std::lock_guard<std::mutex> lock(mtx_infoMap);
    infoMap.clear();
  });

  logger::info("WeaponArt: Loaded {} Weapon Arts.", artMap.size());
}

bool Manager::IsValidWeaponArtID(std::int32_t artID)
{
  if (!Settings::bUseWeaponArtSystem)
    return false;
  return artMap.find(artID) != artMap.end();
}

std::vector<const WeaponArtInfo*> Manager::GetAllWeaponArts()
{
  static std::vector<const WeaponArtInfo*> arts;
  if (!arts.empty())
    return arts;

  arts.reserve(artMap.size());
  for (const auto& [id, art] : artMap)
    arts.push_back(&art);

  std::ranges::sort(arts, [](const WeaponArtInfo* lhs, const WeaponArtInfo* rhs) {
    if (lhs->GetUnlockLevel() != rhs->GetUnlockLevel())
      return lhs->GetUnlockLevel() < rhs->GetUnlockLevel();
    return lhs->GetName() < rhs->GetName();
  });

  return arts;
}

const WeaponArtInfo* Manager::GetWeaponArtInfo(std::int32_t artID)
{
  if (auto it = artMap.find(artID); it != artMap.end())
    return &it->second;
  return nullptr;
}

void Manager::SetWeaponArtInfo(RE::TESObjectWEAP* weapon, std::int32_t artID)
{
  if (!weapon)
    return;

  if (artID == 0) {
    {
      std::lock_guard<std::mutex> lock(mtx_infoMap);
      infoMap.erase(weapon->GetFormID());
    }

    if (auto* player = RE::PlayerCharacter::GetSingleton(); player)
      UpdateWeaponArt(player);
    return;
  }

  auto* art = GetWeaponArtInfo(artID);
  if (!art || !art->IsWeaponAllowed(weapon))
    return;

  {
    std::lock_guard<std::mutex> lock(mtx_infoMap);
    infoMap[weapon->GetFormID()] = artID;
  }

  if (auto* player = RE::PlayerCharacter::GetSingleton(); player)
    UpdateWeaponArt(player);
}

std::int32_t Manager::GetWeaponArtID(const RE::TESObjectWEAP* weapon)
{
  if (!weapon)
    return 0;

  std::lock_guard<std::mutex> lock(mtx_infoMap);
  if (auto it = infoMap.find(weapon->GetFormID()); it != infoMap.end())
    return it->second;
  return 0;
}

std::int32_t Manager::GetActorWeaponArtID(RE::Actor* actor)
{
  if (!actor)
    return 0;

  const auto* left  = actor->GetEquippedObject(true);
  const auto* right = actor->GetEquippedObject(false);

  // 战技必定是右手武器
  if (right && right->IsWeapon())
    if (auto id = GetWeaponArtID(right->As<RE::TESObjectWEAP>()); id != 0)
      return id;

  // 如果左右手有装备，返回无战技
  if (left || right)
    return 0;

  // 到这里说明两只手均为空
  auto race = Race::GetRace(actor);
  switch (race) {
  case Race::Type::Human:
    return "Unarm"_h;  // 人类空手战技ID
  case Race::Type::Werewolf:
    return "Werewolf"_h;  // 狼人空手战技ID
  case Race::Type::Werebear:
    return "Werebear"_h;  // 熊人空手战技ID
  case Race::Type::VampireLord:
    return "VampireLord"_h;  // 吸血鬼领主空手战技ID
  default:
    return 0;
  }
}

void Manager::UpdateWeaponArt(RE::Actor* actor)
{
  if (!actor)
    return;

  auto artID = GetActorWeaponArtID(actor);
  actor->SetGraphVariableInt(ID, artID);

  if (actor->IsPlayerRef())
    UI::WeaponArtHUD::UpdateName(artID);
}

Manager::Perform Manager::GetPerform(RE::Actor* actor)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return Perform::None;

  std::lock_guard<std::mutex> lock(mtx_performCache);
  if (auto it = actorEligibleCache.find(actor); it != actorEligibleCache.end())
    return it->second ? Perform::Eligible : Perform::Subordinate;

  return Perform::None;
}

Manager::State Manager::GetState(RE::Actor* actor)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return State::Disable;

  std::int32_t res = 0;
  if (actor->GetGraphVariableInt(STATE, res))
    return static_cast<State>(res);
  return State::Disable;
}

void Manager::SetState(RE::Actor* actor, State state)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  UpdateWeaponArt(actor);
  // 开关时重置连招状态
  actor->SetGraphVariableInt("MCO_nextattack", 1);
  actor->SetGraphVariableInt("MCO_nextpowerattack", 1);

  actor->SetGraphVariableInt(STATE, static_cast<std::int32_t>(state));

  if (actor->IsPlayerRef())
    UI::WeaponArtHUD::UpdateState(state);
}

void Manager::SwitchWeaponArt(RE::Actor* actor, bool enable)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  auto artID = GetActorWeaponArtID(actor);
  auto art   = GetWeaponArtInfo(artID);
  if (!art)
    return;

  if (enable) {
    if (art->NeedPrepare())
      SetState(actor, State::Prepare);
    else
      SetState(actor, State::Enable);
  } else {
    SetState(actor, State::Disable);
  }
}

void Manager::Stage(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  auto stageID = Utils::toInt(payload);
  if (!stageID) {
    logger::warn("Invalid stage ID: {}.", payload);
    return;
  }

  auto artID = GetActorWeaponArtID(actor);
  auto art   = GetWeaponArtInfo(artID);
  if (!art) {
    logger::warn("Actor {} does not have a valid Weapon Art. Cannot start stage {}.",
                 actor->GetName(), stageID.value());
    return;
  }

  auto stage = art->GetStageData(stageID.value());

  if (!stage) {
    logger::warn("Invalid stage ID {} for Weapon Art {}.", stageID.value(),
                 art ? art->GetName() : "Unknown");
    return;
  }

  float manaCost = stage->manaCost;
  float minMana  = stage->minMana;

  auto currentMana = actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kMagicka);
  {
    std::scoped_lock lock(mtx_performCache);
    actorEligibleCache[actor] = currentMana >= minMana;
  }

  if (currentMana >= minMana)
    actor->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kMagicka, manaCost);

  // Payload优化
  Stamina::Start(actor);
}

void Manager::Attack(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 2) {
    logger::warn("Invalid attack payload '{}'. Expected format 'stageID|attackID'.", payload);
    return;
  }

  auto stageID  = Utils::toInt(split[0]);
  auto attackID = Utils::toInt(split[1]);

  if (!stageID || !attackID) {
    logger::warn("Invalid stage ID {} or Invalid attack ID {}", split[0], split[1]);
    return;
  }

  auto artID = GetActorWeaponArtID(actor);
  auto art   = GetWeaponArtInfo(artID);
  if (!art) {
    logger::warn("Actor {} does not have a valid Weapon Art. Cannot perform attack {}.",
                 actor->GetName(), payload);
    return;
  }

  auto attack = art->GetAttackData(stageID.value(), attackID.value());

  if (!attack) {
    logger::warn("Invalid attack ID {} for stage ID {} in Weapon Art {}.", attackID.value(),
                 stageID.value(), art ? art->GetName() : "Unknown");
    return;
  }

  auto side = Stamina::Side::None;

  if (attack->left == attack->right)
    side = Stamina::Side::Auto;
  else if (attack->left)
    side = Stamina::Side::Left;
  else if (attack->right)
    side = Stamina::Side::Right;
  else
    side = Stamina::Side::Auto;

  if (GetPerform(actor) == Perform::Eligible) {
    Stamina::RimStaminaConsume(actor, side, attack->powerAttack, attack->staminaMult);
    Damage::SetMult(actor, attack->damageMult);
    Poise::TargetSet(actor, attack->poiseDamageMult);
    Posture::TargetSet(actor, attack->postureDamageMult);
  } else {
    Stamina::RimStaminaConsume(actor, side, attack->powerAttack, attack->subStaminaMult);
    Damage::SetMult(actor, attack->subDamageMult);
    Poise::TargetSet(actor, attack->subPoiseDamageMult);
    Posture::TargetSet(actor, attack->subPostureDamageMult);
  }
}

void Manager::End(RE::Actor* actor)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  std::scoped_lock lock(mtx_performCache);
  actorEligibleCache.erase(actor);

  // Payload优化
  Damage::End(actor);
  Poise::End(actor);
  Poise::TargetEnd(actor);
  Posture::TargetEnd(actor);
  Stagger::TargetEnd(actor);
  Stamina::End(actor);
}

void Manager::Cast(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  if (GetPerform(actor) != Perform::Eligible)
    return;
  auto spellHash = Utils::hash(payload);
  auto artId     = GetActorWeaponArtID(actor);
  auto art       = GetWeaponArtInfo(artId);

  auto spellInfo = art ? art->GetSpellInfo(spellHash) : std::nullopt;
  if (!spellInfo) {
    logger::warn("Spell name {} not found for Weapon Art {}.", payload,
                 art ? art->GetName() : "Unknown");
    return;
  }

  auto [skillLevel, skillMult] = GetSkillData(actor, spellInfo->skill);

  // 防止技能等级为0导致的倍率异常
  if (skillLevel == 0.0f)
    skillLevel = 1.0f;

  // 首先根据全局的技能百分比变化决定基础倍率，再根据技能等级调整伤害，最后乘以法术定义的倍率
  float magnitude = spellInfo->stdMagnitude * skillMult;

  // 标准强度25级为基准，在25级倍率刚好为1
  // factor为0时，倍率恒为1；factor为正时，等级越高倍率越大
  float exponent = spellInfo->factor / (spellInfo->factor + 10.0f);
  float scale    = std::pow(skillLevel / 25.0f, exponent);

  // 最小倍率为0.1，防止过于悬殊的伤害
  if (scale < 0.1f)
    scale = 0.1f;

  // CastSpellImmediate不会在再计算一次skillMult
  auto caster = actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
  // 法术，是否不显示特效，作用对象，？，是否造成敌意，施法强度，责任对象
  caster->CastSpellImmediate(spellInfo->spell, false, spellInfo->selfCast ? actor : nullptr,
                             spellInfo->effectiveness, true, magnitude * scale, actor);
}

void Manager::PayloadParse(RE::Actor* actor, const std::string& payload)
{

  if (payload.starts_with("stage|"))
    Stage(actor, payload.substr(6));
  else if (payload.starts_with("attack|"))
    Attack(actor, payload.substr(7));
  else if (payload == "end")
    End(actor);
  else if (payload == "prepareend")
    WeaponArt::Manager::SetState(actor, WeaponArt::Manager::State::Enable);
  else if (payload == "toprepare")
    WeaponArt::Manager::SetState(actor, WeaponArt::Manager::State::Prepare);
  else if (payload.starts_with("cast|"))
    Cast(actor, payload.substr(5));
}

void Manager::Interrupt(RE::Actor* actor)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  std::lock_guard lock(mtx_performCache);
  actorEligibleCache.erase(actor);
}
}  // namespace WeaponArt
