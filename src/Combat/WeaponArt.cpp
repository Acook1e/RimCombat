#include "Combat/WeaponArt.h"

#include "Combat/Damage.h"
#include "Combat/Posture.h"
#include "Combat/Stamina.h"
#include "GUI/UI.h"

#include "magic_enum/magic_enum_flags.hpp"
#include "nlohmann/json.hpp"

namespace WeaponArt
{

using AvailableWeaponType = WeaponArtInfo::AvailableWeaponType;
using AvailableWeapon     = WeaponArtInfo::AvailableWeapon;

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

AvailableWeapon WeaponTypeMapping(Weapon::Type type)
{
  using AW = AvailableWeapon;

  switch (type) {
  // ── 不参与通用战技 ──
  case Weapon::Type::None:
  case Weapon::Type::Unarm:
  case Weapon::Type::Werewolf:
  case Weapon::Type::Werebear:
  case Weapon::Type::VampireLord:
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
                             const std::vector<RE::FormID>& weapons, std::uint8_t consumePoint,
                             std::uint8_t unlockLevel, bool needPrepare)
{
  this->id              = id;
  this->name            = std::move(name);
  this->description     = std::move(description);
  this->availableWeapon = availableWeapon;
  this->weapons         = std::move(weapons);
  this->consumePoint    = consumePoint;
  this->unlockLevel     = unlockLevel;
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

  auto aviailableAttack = availableWeapon & attackMask;

  // 如果没有指定攻击类型要求，则视为通配
  if (aviailableAttack == AvailableWeapon::None)
    return (availableWeapon | attackMask) >= weaponType;

  // 如果指定了攻击类型要求，则武器的攻击类型必须满足要求
  auto weaponAttack = weaponType & attackMask;
  return (availableWeapon | attackMask) >= (weaponType | attackMask) &&
         weaponAttack >= aviailableAttack;
}

bool PlayerStat::IsUnlocked(std::int32_t artID)
{
  return unlockedArts.find(artID) != unlockedArts.end();
}

PlayerStat::PlayerStat()
{
  Serialization::RegisterSaveCallback(serialType, [](SKSE::SerializationInterface* serial) {
    serial->WriteRecordData(&exp, sizeof(exp));
    serial->WriteRecordData(&level, sizeof(level));
    serial->WriteRecordData(&point, sizeof(point));

    auto count = static_cast<std::uint32_t>(unlockedArts.size());
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

    std::uint32_t count = 0;
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
    unlockedArts.clear();
  });
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
    RE::DebugMessageBox(std::format("WeaponArt Level Up, Current Level: {}!", level).data());
    // 抵达上限后等级不会增加，但仍然可以获得经验和战技点数
    if (level >= 100)
      level = 100;
  }
}

bool PlayerStat::UnlockArt(std::int32_t artID)
{
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
  // 从JSON文件加载战技信息
  const std::string weaponArtDir = std::string(Settings::SettingsDir) + "WeaponArt/";
  for (const auto& entry : std::filesystem::directory_iterator(weaponArtDir)) {
    if (entry.is_regular_file() && entry.path().extension() == ".json") {
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
          std::string name        = value.at("name").get<std::string>();
          std::string description = value.at("description").get<std::string>();

          std::vector<std::string> weaponStrs = value.at("weapons").get<std::vector<std::string>>();
          std::vector<std::string> availableWeaponStrs =
              value.at("availableWeapon").get<std::vector<std::string>>();

          std::vector<RE::FormID> weapons{};
          auto dataHandle = RE::TESDataHandler::GetSingleton();
          for (const auto& w : weaponStrs) {
            auto split = Utils::split(w, '|');
            if (split.size() != 2) {
              logger::warn("Invalid weapon '{}' for Weapon Art {} in file {}. Skipping.", w, key,
                           entry.path().string());
              continue;
            }
            auto res = dataHandle->LookupFormID(Utils::hash(split[0]), split[1]);
            if (res)
              weapons.push_back(res);
          }

          AvailableWeapon availableWeapon = AvailableWeapon::None;
          for (const auto& awStr : availableWeaponStrs) {
            if (auto awOpt = magic_enum::enum_flags_cast<AvailableWeapon>(awStr);
                awOpt.has_value()) {
              availableWeapon = availableWeapon | awOpt.value();
            } else {
              logger::warn("Invalid weapon art type '{}' for Weapon Art {} in file {}. Skipping.",
                           awStr, key, entry.path().string());
            }
          }
          // 如果不是Unique类型，则需要后处理保证匹配
          if (availableWeapon != AvailableWeapon::Unique) {
            // 如果没有指定重量要求，则视为通配
            if ((availableWeapon & weightMask) == AvailableWeapon::None)
              availableWeapon = availableWeapon | weightMask;
            // 如果没有指定武器家族要求，则视为通配
            if ((availableWeapon & familyMask) == AvailableWeapon::None)
              availableWeapon = availableWeapon | familyMask;
          }

          std::uint8_t consumePoint = value.at("consumePoint").get<std::uint8_t>();
          std::uint8_t unlockLevel  = value.at("unlockLevel").get<std::uint8_t>();
          bool needPrepare          = value.at("needPrepare").get<bool>();

          // 如果想要查看ID，可以在对应的战技定义中添加"verbose": true字段
          bool verbose = value.value("verbose", false);
          if (verbose)
            logger::info("Loaded Weapon Art {} (ID: {}) from file {}.", name, id,
                         entry.path().string());

          WeaponArtInfo art(id, name, description, availableWeapon, weapons, consumePoint,
                            unlockLevel, needPrepare);
          artMap[id] = std::move(art);
        }
      } catch (const std::exception& e) {
        logger::error("Failed to load Weapon Art from file {}: {}", entry.path().string(),
                      e.what());
      }
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

  // 反转时直接清空数据
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    std::lock_guard<std::mutex> lock(mtx_infoMap);
    infoMap.clear();
  });
}

bool Manager::IsValidWeaponArtID(std::int32_t artID)
{
  if (!Settings::bUseWeaponArtSystem)
    return false;

  std::lock_guard<std::mutex> lock(mtx_infoMap);
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
  std::lock_guard<std::mutex> lock(mtx_infoMap);
  if (auto it = artMap.find(artID); it != artMap.end())
    return &it->second;
  return nullptr;
}

void Manager::SetWeaponArtInfo(RE::TESObjectWEAP* weapon, std::int32_t artID)
{
  if (!weapon)
    return;

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
  auto type = Weapon::GetActorEquipmentType(actor, false);
  switch (type) {
  case Weapon::Type::Unarm:
    return "Unarm"_h;  // 人类空手战技ID
  case Weapon::Type::Werewolf:
    return "Werewolf"_h;  // 狼人空手战技ID
  case Weapon::Type::Werebear:
    return "Werebear"_h;  // 熊人空手战技ID
  case Weapon::Type::VampireLord:
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

void Manager::Start(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  auto manaCostStr = payload.substr(6);
  auto split       = Utils::split(manaCostStr, '|');
  if (split.size() != 2)
    return;
  float manaCost = Utils::toFloat(split[0]);
  float minMana  = Utils::toFloat(split[1]);
  if (manaCost < 0.0f || minMana < 0.0f)
    return;
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

void Manager::End(RE::Actor* actor)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  std::scoped_lock lock(mtx_performCache);
  actorEligibleCache.erase(actor);

  // Payload优化
  Stamina::End(actor);
  Damage::End(actor);
  Posture::End(actor);
}

void Manager::Cast(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  if (GetPerform(actor) != Perform::Eligible)
    return;
  auto paramsStr = payload.substr(5);
  auto split     = Utils::split(paramsStr, '|');
  if (split.size() != 5)
    return;

  std::string modName = split[0];
  RE::FormID formID   = Utils::toInt(split[1]);

  auto dataHandle      = RE::TESDataHandler::GetSingleton();
  RE::SpellItem* spell = dataHandle->LookupForm<RE::SpellItem>(formID, modName);
  RE::Actor* target    = split[2] == "true" ? actor : nullptr;
  float effectiveness  = Utils::toFloat(split[3]);
  float magnitude      = Utils::toFloat(split[4]);
  if (!spell || effectiveness < 0.0f || magnitude < 0.0f)
    return;

  auto caster = actor->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant);
  // 法术，是否不显示特效，作用对象，？，是否造成敌意，施法强度，责任对象
  caster->CastSpellImmediate(spell, false, target, effectiveness, true, magnitude, actor);
}

void Manager::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload.starts_with("start|"))
    Start(actor, payload);
  else if (payload == "end")
    End(actor);
  else if (payload == "prepareend")
    WeaponArt::Manager::SetState(actor, WeaponArt::Manager::State::Enable);
  else if (payload == "toprepare")
    WeaponArt::Manager::SetState(actor, WeaponArt::Manager::State::Prepare);
  else if (payload.starts_with("cast|"))
    Cast(actor, payload);
}

void Manager::Interrupt(RE::Actor* actor)
{
  if (!actor || !Settings::bUseWeaponArtSystem)
    return;

  std::lock_guard lock(mtx_performCache);
  actorEligibleCache.erase(actor);
}
}  // namespace WeaponArt
