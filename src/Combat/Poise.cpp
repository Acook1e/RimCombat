#include "Combat/Poise.h"

#include "Combat/Execution.h"
#include "Combat/Stagger.h"
#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Race.h"
#include "Data/Weapon.h"
#include "Utils.h"

using PoiseData = Poise::PoiseData;

void Poise::InitPoiseData(RE::Actor* actor)
{
  // 内部无锁调用

  float maxPoise      = CalculateMaxPoise(actor);
  poiseDataMap[actor] = {maxPoise, maxPoise, 0};
}

float Poise::CalculateMaxPoise(RE::Actor* actor)
{
  // 不涉及到任何需要锁的资源

  if (!actor)
    return 0.0f;

  float base = Race::GetBasePoiseHealth(actor);

  auto race = actor->GetRace()->GetFormID();
  if (racePoiseOverrideMap.contains(race))
    base = racePoiseOverrideMap[race];

  auto actorFormID = actor->GetFormID();
  if (!actor->GetActorBase()->IsUnique())
    actorFormID = actor->GetActorBase()->GetFormID();
  if (actorPoiseOverrideMap.contains(actorFormID))
    base = actorPoiseOverrideMap[actorFormID];

  float mass       = actor->GetRace()->data.baseMass;
  float maxStamina = Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kStamina);
  float armorBonus = 0.0f;

  // 获取所有装备的护甲类型对应的韧性加成
  for (auto& [obj, pair] : actor->GetInventory([](RE::TESBoundObject& object) {
         return object.IsArmor();
       })) {
    auto armor = obj->As<RE::TESObjectARMO>();
    if (armor->IsLightArmor()) {
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kHead))
        armorBonus += Settings::fLightArmorHeadMaxPoiseBonus;
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kBody))
        armorBonus += Settings::fLightArmorBodyMaxPoiseBonus;
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kHands))
        armorBonus += Settings::fLightArmorHandMaxPoiseBonus;
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kFeet))
        armorBonus += Settings::fLightArmorFeetMaxPoiseBonus;
    } else if (armor->IsHeavyArmor()) {
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kHead))
        armorBonus += Settings::fHeavyArmorHeadMaxPoiseBonus;
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kBody))
        armorBonus += Settings::fHeavyArmorBodyMaxPoiseBonus;
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kHands))
        armorBonus += Settings::fHeavyArmorHandMaxPoiseBonus;
      if (armor->HasPartOf(RE::BIPED_MODEL::BipedObjectSlot::kFeet))
        armorBonus += Settings::fHeavyArmorFeetMaxPoiseBonus;
    }
  }

  float maxPoise = base + maxStamina * Settings::fPoiseStaminaMult +
                   mass * Settings::fPoiseMassMult + armorBonus;
  return maxPoise;
}

PoiseData& Poise::GetPoiseDataRef(RE::Actor* actor)
{
  // 内部无锁调用

  if (poiseDataMap.contains(actor))
    return poiseDataMap[actor];
  InitPoiseData(actor);
  return poiseDataMap[actor];
}

Poise::Poise()
{
  // 注册序列化
  Serialization::RegisterSaveCallback(serialType, [](SKSE::SerializationInterface* serial) {
    std::lock_guard<std::mutex> lock(mtx_poiseData);
    std::unordered_map<std::uint64_t, PoiseData> persistMap;
    for (const auto& [actor, data] : poiseDataMap) {
      auto persist = Serialization::ToPersistForm(actor->GetFormID());
      if (persist)
        persistMap[persist] = data;
    }

    std::uint32_t count = static_cast<std::uint32_t>(persistMap.size());
    serial->WriteRecordData(&count, sizeof(count));
    for (const auto& [persist, data] : persistMap) {
      serial->WriteRecordData(&persist, sizeof(persist));
      serial->WriteRecordData(&data.current, sizeof(data.current));
      serial->WriteRecordData(&data.max, sizeof(data.max));
    }
  });

  Serialization::RegisterLoadCallback(serialType, [](SKSE::SerializationInterface* serial) {
    std::lock_guard<std::mutex> lock(mtx_poiseData);
    poiseDataMap.clear();

    std::uint32_t count;
    if (serial->ReadRecordData(&count, sizeof(count))) {
      for (std::uint32_t i = 0; i < count; ++i) {
        std::uint64_t persist;
        PoiseData data{0.0f, 0.0f, 0};
        if (serial->ReadRecordData(&persist, sizeof(persist)) &&
            serial->ReadRecordData(&data.current, sizeof(data.current)) &&
            serial->ReadRecordData(&data.max, sizeof(data.max))) {
          auto formID = Serialization::ToForm(persist);
          if (!formID)
            continue;
          auto form = RE::TESForm::LookupByID(formID);
          if (auto actor = form ? form->As<RE::Actor>() : nullptr; actor)
            poiseDataMap[actor] = std::move(data);
        }
      }
    }
  });

  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    {
      std::scoped_lock lock(mtx_poiseData);
      poiseDataMap.clear();
    }
    {
      std::scoped_lock lock(mtx_poiseMultiplier);
      poiseMultOnHit.clear();
      poiseMultSelf.clear();
    }
  });

  logger::info("Poise system initialized");
}

void Poise::Update(std::uint64_t deltaTime)
{
  if (!Settings::bUsePoiseSystem)
    return;

  auto now = Utils::GetTime<std::chrono::milliseconds>();

  // 更新韧性数据，处理韧性值的自然恢复
  std::lock_guard<std::mutex> lock(mtx_poiseData);
  for (auto& [actor, data] : poiseDataMap) {
    if (now < data.regenResumeTime)
      continue;
    if (data.current >= data.max)
      continue;
    data.current +=
        Settings::fPoiseRegenPercentPerSecond * (deltaTime / 1000.0f) * (data.max / 100.0f);
    if (data.current > data.max)
      data.current = data.max;
  }
}

void Poise::UpdateMaxPoise(RE::Actor* actor)
{
  if (!actor || !Settings::bUsePoiseSystem)
    return;

  float maxPoise = CalculateMaxPoise(actor);
  std::lock_guard<std::mutex> lock(mtx_poiseData);
  auto& data = GetPoiseDataRef(actor);
  data.max   = maxPoise;

  // 如果当前韧性值超过新的上限，则直接降到上限
  if (data.current > data.max)
    data.current = data.max;
}

Stagger::Level Poise::CalculateStaggerLevel(float poiseDamage)
{
  if (poiseDamage >= Settings::fStaggerLevelLarge)
    return Stagger::Level::Large;
  else if (poiseDamage >= Settings::fStaggerLevelMedium)
    return Stagger::Level::Medium;
  else if (poiseDamage >= Settings::fStaggerLevelSmall)
    return Stagger::Level::Small;
  return Stagger::Level::None;
}

PoiseData Poise::GetPoiseData(RE::Actor* actor)
{
  std::lock_guard<std::mutex> lock(mtx_poiseData);
  if (poiseDataMap.contains(actor))
    return poiseDataMap[actor];
  InitPoiseData(actor);
  return poiseDataMap[actor];
}

void Poise::ProcessWeaponHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData)
{
  if (!aggressor || !victim || !Settings::bUsePoiseSystem)
    return;
  if (victim->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;
  if (hitData.totalDamage <= 0.0f)
    return;

  // 如果攻击被格挡了，则不处理韧性
  if (hitData.flags.any(RE::HitData::Flag::kBlocked))
    return;

  auto attackWeapon = hitData.weapon;

  auto bash   = hitData.flags.any(RE::HitData::Flag::kBash);
  auto shield = false;
  if (auto* left = aggressor->GetEquippedObject(true); left)
    shield = left->IsArmor();

  float base = 0.0f;

  if (!attackWeapon) {
    if (bash) {
      auto type = Weapon::Type::None;
      if (shield)
        type = Weapon::Type::Shield;
      else
        type = Weapon::GetActorEquipmentType(aggressor, false);
      base = Weapon::GetBasePoiseDamage(type);
    } else {
      // 武器为空且不是Bash，说明攻击来源于生物
      auto race = Race::GetRace(aggressor);
      base      = Race::GetBasePoiseDamage(race);
    }
  } else {
    auto type = Weapon::GetWeaponType(attackWeapon);
    base      = Weapon::GetBasePoiseDamage(type);
  }

  auto aggressorEntry = RE::BGSEntryPoint::ENTRY_POINTS::kModTargetStagger;
  auto victimEntry    = RE::BGSEntryPoint::ENTRY_POINTS::kModIncomingStagger;

  // 暂时不使用任何原版的entry
  // 因为无法判断是乘法还是加法

  // 根据类型应用不同的韧性伤害倍率
  float poiseDamage = base;
  if (hitData.flags.any(RE::HitData::Flag::kPowerAttack)) {
    if (bash)
      poiseDamage *= Settings::fPowerBashPoiseDamageMult;
    else
      poiseDamage *= Settings::fPowerAttackPoiseDamageMult;
  } else if (bash) {
    poiseDamage *= Settings::fBashPoiseDamageMult;
  }

  // 根据缓存应用韧性伤害倍率调整
  {
    std::scoped_lock lock(mtx_poiseMultiplier);
    if (auto iter = poiseMultOnHit.find(aggressor); iter != poiseMultOnHit.end())
      poiseDamage *= iter->second;
    if (auto iter = poiseMultSelf.find(victim); iter != poiseMultSelf.end())
      poiseDamage *= iter->second;
  }

  DamagePoiseHealth(victim, poiseDamage);
}

void Poise::DamagePoiseHealth(RE::Actor* actor, float value)
{
  if (!actor || value <= 0.0f || !Settings::bUsePoiseSystem)
    return;

  // 处理韧性伤害
  // 保证锁的连贯性，避免多次伤害导致的硬直等级计算时序问题
  std::lock_guard<std::mutex> lock(mtx_poiseData);
  auto& data = GetPoiseDataRef(actor);
  data.current -= value;
  data.regenResumeTime = Utils::GetTime<std::chrono::milliseconds>() + Settings::uPoiseRegenDelay;

  // 对于处于处决状态的目标，直接根据伤害值触发对应等级更高一级的硬直
  if (Execution::IsExecutable(actor)) {
    auto level = CalculateStaggerLevel(value);
    if (level < Stagger::Level::Large) {
      auto newLevel     = static_cast<Stagger::Level>(static_cast<std::uint8_t>(level) + 1);
      auto currentLevel = Stagger::GetStaggerLevel(actor);
      if (newLevel > currentLevel)
        Stagger::SetStaggerLevel(actor, newLevel);
    } else
      Stagger::SetStaggerLevel(actor, Stagger::Level::Knockaway);

    // 重置韧性值，避免重复触发硬直
    data.current = data.max;
    Execution::ExitExecutable(actor);
    return;
  }

  // 硬直补偿机制，如果一次削韧够大但又没有削到0，触发低一级别的硬直以补偿玩家的打击感
  bool staggerCompensation = value / data.max >= Settings::fStaggerCompensationPercent;

  if (data.current > 0.0f && !staggerCompensation)
    return;

  data.current = data.max;

  // 到这部分说明韧性值已经被打破，触发硬直
  auto level = CalculateStaggerLevel(value);
  if (staggerCompensation && level > Stagger::Level::Small)
    level = static_cast<Stagger::Level>(static_cast<std::uint8_t>(level) - 1);
  auto currentLevel = Stagger::GetStaggerLevel(actor);
  if (level != Stagger::Level::None && level > currentLevel)
    Stagger::SetStaggerLevel(actor, level);
}

void Poise::Set(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUsePoiseSystem)
    return;

  auto multiplier = Utils::toFloat(payload);
  if (multiplier < 0.0f)
    return;
  std::lock_guard<std::mutex> lock(mtx_poiseMultiplier);
  poiseMultSelf[actor] = multiplier;
}

void Poise::End(RE::Actor* actor)
{
  if (!actor || !Settings::bUsePoiseSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_poiseMultiplier);
  poiseMultSelf.erase(actor);
}

void Poise::TargetSet(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUsePoiseSystem)
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 1 && split.size() != 2)
    return;

  auto multiplier = Utils::toFloat(split[0]);
  auto fallback   = split.size() == 2 ? Utils::toFloat(split[1]) : multiplier;
  if (multiplier < 0.0f || fallback < 0.0f)
    return;

  auto perform = WeaponArt::Manager::GetPerform(actor);
  if (perform == WeaponArt::Manager::Perform::Subordinate)
    multiplier = fallback;

  std::lock_guard<std::mutex> lock(mtx_poiseMultiplier);
  poiseMultOnHit[actor] = multiplier;
}

void Poise::TargetEnd(RE::Actor* actor)
{
  if (!actor || !Settings::bUsePoiseSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_poiseMultiplier);
  poiseMultOnHit.erase(actor);
}

void Poise::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUsePoiseSystem)
    return;

  if (payload.starts_with("set|"))
    Set(actor, payload.substr(4));
  else if (payload == "end")
    End(actor);
  else if (payload.starts_with("targetset|"))
    TargetSet(actor, payload.substr(10));
  else if (payload == "targetend")
    TargetEnd(actor);
}