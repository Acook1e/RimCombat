#include "Combat/WeaponArt.h"

#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

namespace WeaponArt
{

using AvailableWeapon = WeaponArtInfo::AvailableWeapon;
using DamageType      = WeaponArtInfo::DamageType;

WeaponArtInfo::WeaponArtInfo(std::int32_t id, const std::string& name,
                             const std::string& description,
                             AvailableWeapon availableWeapon,
                             const std::vector<RE::FormID>& weapons,
                             DamageType damageType, float damageMult,
                             float baseDamage, float postureDamageMult,
                             std::uint8_t consumePoint,
                             std::uint8_t unlockLevel)
{
  this->id                = id;
  this->name              = std::move(name);
  this->description       = std::move(description);
  this->availableWeapon   = availableWeapon;
  this->weapons           = std::move(weapons);
  this->damageType        = damageType;
  this->damageMult        = damageMult;
  this->baseDamage        = baseDamage;
  this->postureDamageMult = postureDamageMult;
  this->consumePoint      = consumePoint;
  this->unlockLevel       = unlockLevel;
}

inline AvailableWeapon operator|(AvailableWeapon lhs, AvailableWeapon rhs)
{
  auto l = static_cast<std::uint16_t>(lhs);
  auto r = static_cast<std::uint16_t>(rhs);
  return static_cast<AvailableWeapon>(l | r);
}

inline bool operator>=(AvailableWeapon lhs, AvailableWeapon rhs)
{
  auto l = static_cast<std::uint16_t>(lhs);
  auto r = static_cast<std::uint16_t>(rhs);
  return (l & r) == r;
}

AvailableWeapon WeaponTypeMapping(Weapon::Type type)
{
  switch (type) {
  case Weapon::Type::Unarm:
  case Weapon::Type::Werewolf:
  case Weapon::Type::VampireLord:
    // 不应该执行到这里，因为空手武器不应该有战技信息，但为了安全起见，返回一个无效的可用武器类型
    return AvailableWeapon::Unique;
  case Weapon::Type::Dagger:
    return AvailableWeapon::LightWeapon | AvailableWeapon::Sword;
  case Weapon::Type::Sword:
    return AvailableWeapon::NormalWeapon | AvailableWeapon::SlashWeapon |
           AvailableWeapon::Sword;
  case Weapon::Type::Axe:
    return AvailableWeapon::NormalWeapon | AvailableWeapon::SlashWeapon |
           AvailableWeapon::Axe;
  case Weapon::Type::Mace:
    return AvailableWeapon::NormalWeapon | AvailableWeapon::StrikeWeapon |
           AvailableWeapon::Hammer;
  case Weapon::Type::GreatSword:
    return AvailableWeapon::HeavyWeapon | AvailableWeapon::SlashWeapon |
           AvailableWeapon::Sword;
  case Weapon::Type::GreatAxe:
    return AvailableWeapon::HeavyWeapon | AvailableWeapon::SlashWeapon |
           AvailableWeapon::Axe;
  case Weapon::Type::GreatMace:
    return AvailableWeapon::HeavyWeapon | AvailableWeapon::StrikeWeapon |
           AvailableWeapon::Hammer;
  case Weapon::Type::Bow:
  case Weapon::Type::Crossbow:
    return AvailableWeapon::RangeWeapon | AvailableWeapon::Bow;
  default:
    return AvailableWeapon::
        Unique;  // 默认返回Unique，表示不符合任何战技使用条件
  }
}

bool WeaponArtInfo::IsWeaponAllowed(RE::TESObjectWEAP* weapon) const
{
  if (!weapon)
    return false;

  // 优先检查Unique条件
  if (availableWeapon == AvailableWeapon::Unique)
    return std::find(weapons.begin(), weapons.end(), weapon->GetFormID()) !=
           weapons.end();

  // 检查其他可用武器类型
  auto weaponType = WeaponTypeMapping(Weapon::GetWeaponType(weapon));
  return (weaponType >= availableWeapon);
}

PlayerStat::PlayerStat()
{
  Serialization::RegisterSaveCallback(
      WeaponExp, [](SKSE::SerializationInterface* serial) {
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

  Serialization::RegisterLoadCallback(
      WeaponExp, [](SKSE::SerializationInterface* serial) {
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

  Serialization::RegisterRevertCallback(WeaponExp,
                                        [](SKSE::SerializationInterface*) {
                                          exp   = 0.0f;
                                          level = 1;
                                          point = 0;
                                          unlockedArts.clear();
                                        });
}

void PlayerStat::AddExp(float value)
{
  if (value <= 0.0f)
    return;

  exp += value;

  // 升级所需经验公式：100 * 当前等级
  float requiredExp = 100.0f * level;
  while (exp >= requiredExp) {
    exp -= requiredExp;
    level++;
    point += 1;  // 每升一级获得1点战技点数
    requiredExp = 100.0f * level;
  }
}

bool PlayerStat::UnlockArt(const WeaponArtInfo& art)
{
  if (art.GetUnlockLevel() > level)  // 等级不足
    return false;
  if (art.GetConsumePoint() > point)  // 战技点数不足
    return false;

  // 解锁战技
  if (unlockedArts.find(art.GetID()) != unlockedArts.end())
    return true;  // 已解锁

  point -= art.GetConsumePoint();
  unlockedArts.insert(art.GetID());
  return true;
}

Manager::Manager()
{
  // 从JSON文件加载战技信息
  const std::string weaponArtDir =
      std::string(Settings::SettingsDir) + "WeaponArt/";
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
            logger::warn("Invalid Weapon Art ID for {} in file {}. Skipping.",
                         key, entry.path().string());
            continue;
          }
          // 一般hash值不会发生碰撞，但为了安全起见，仍然验证ID的唯一性
          if (artMap.find(id) != artMap.end()) {
            logger::warn(
                "Hash collision for Weapon Art {} in file {}. Skipping.", key,
                entry.path().string());
            continue;
          }
          std::string name        = value.at("name").get<std::string>();
          std::string description = value.at("description").get<std::string>();

          // 测试阶段先跳过
          std::vector<RE::FormID> weapons{};
          AvailableWeapon availableWeapon =
              (std::numeric_limits<AvailableWeapon>::max)();
          DamageType damageType = DamageType::None;

          float damageMult        = value.at("damageMult").get<float>();
          float baseDamage        = value.at("baseDamage").get<float>();
          float postureDamageMult = value.at("postureDamageMult").get<float>();
          std::uint8_t consumePoint =
              value.at("consumePoint").get<std::uint8_t>();
          std::uint8_t unlockLevel =
              value.at("unlockLevel").get<std::uint8_t>();

          // 输出日志以验证ID
          logger::info("Loaded Weapon Art {} (ID: {}) from file {}.", name, id,
                       entry.path().string());

          WeaponArtInfo art(id, name, description, availableWeapon, weapons,
                            damageType, damageMult, baseDamage,
                            postureDamageMult, consumePoint, unlockLevel);
          artMap[id] = std::move(art);
        }
      } catch (const std::exception& e) {
        logger::error("Failed to load Weapon Art from file {}: {}",
                      entry.path().string(), e.what());
      }
    }
  }

  // 测试阶段跳过序列化
  // Serialization::RegisterSaveCallback(weaponInfo,
  // [](SKSE::SerializationInterface* serial) {  });
}
}  // namespace WeaponArt
