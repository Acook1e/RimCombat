#include "Combat/Posture.h"

#include "Combat/Block.h"
#include "Combat/Execution.h"
#include "Combat/Exhausted.h"
#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Race.h"
#include "Data/Weapon.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"

float Posture::InitPosture(RE::Actor* actor)
{
  // 仅在获取值时初始化架势数据，避免不必要的初始化开销
  // 因此可以无锁

  float maxPosture  = CalculateMaxPosture(actor);
  postureMap[actor] = {maxPosture, maxPosture, 0};
  return maxPosture;
}

float Posture::CalculateMaxPosture(RE::Actor* actor)
{
  if (!actor)
    return 0.0f;

  float base = Race::GetBasePostureHealth(actor);

  auto race = actor->GetRace()->GetFormID();
  if (racePostureOverride.contains(race))
    base = racePostureOverride[race];

  auto actorFormID = actor->GetFormID();
  // 对于非独特NPC，获取到的FormID为临时生成的
  // 因此需要使用ActorBase的FormID来获取覆盖值
  if (!actor->GetActorBase()->IsUnique())
    actorFormID = actor->GetActorBase()->GetFormID();
  // 对于特定NPC的覆盖优先级高于种族覆盖
  if (actorPostureOverride.contains(actorFormID))
    base = actorPostureOverride[actorFormID];

  float maxHealth  = Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kHealth);
  float maxStamina = Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kStamina);
  float maxPosture = base + maxHealth * Settings::fMaxPostureHealthMult +
                     maxStamina * Settings::fMaxPostureStaminaMult;
  return maxPosture;
}

Posture::PostureData& Posture::GetPostureDataRef(RE::Actor* actor)
{
  // 无锁，仅用于内部调用
  if (postureMap.contains(actor))
    return postureMap[actor];
  InitPosture(actor);
  return postureMap[actor];
}

Posture::Posture()
{
  Serialization::RegisterSaveCallback(serialType, [](SKSE::SerializationInterface* serial) {
    // 将FormID转换为持久化格式
    // 并自动去除非法或未找到的FormID
    std::unordered_map<std::uint64_t, PostureData> persistMap;
    for (const auto& [actor, data] : postureMap) {
      auto persist = Serialization::ToPersistForm(actor->GetFormID());
      if (persist)
        persistMap[persist] = std::move(data);
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
    std::unique_lock lock(mtx_postureData);
    postureMap.clear();

    std::uint32_t count;
    if (serial->ReadRecordData(&count, sizeof(count))) {
      for (std::uint32_t i = 0; i < count; ++i) {
        std::uint64_t persist;
        PostureData data;
        if (serial->ReadRecordData(&persist, sizeof(persist)) &&
            serial->ReadRecordData(&data.current, sizeof(data.current)) &&
            serial->ReadRecordData(&data.max, sizeof(data.max))) {
          auto formID = Serialization::ToForm(persist);
          if (!formID)
            continue;
          auto form = RE::TESForm::LookupByID(formID);
          if (auto actor = form ? form->As<RE::Actor>() : nullptr; actor)
            postureMap[actor] = std::move(data);
        }
      }
    }
  });

  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    {
      std::unique_lock lock(mtx_postureData);
      postureMap.clear();
    }
    {
      std::scoped_lock lock(mtx_damageCache);
      damageCache.clear();
    }
    {
      std::scoped_lock lock(mtx_unbreakableCache);
      unbreakableActors.clear();
    }
  });

  logger::info("Posture system initialized");
}

void Posture::Update(std::uint64_t deltaTime)
{
  auto now = Utils::GetTime<std::chrono::milliseconds>();

  // 架势恢复
  {
    std::unique_lock lock(mtx_postureData);
    for (auto& [actor, data] : postureMap) {
      if (data.current < data.max) {
        if (Execution::IsExecutable(actor)) {
          // 作为平衡性和视觉表现上的优化
          // 进入处决状态默认恢复到一半的最大值，并在处决状态内以0.2倍速度恢复
          data.current +=
              data.max * (Settings::fPostureRegenPercentPerSecond / 100.0f) * (deltaTime / 5000.0f);
          data.current = std::clamp(data.current, 0.0f, data.max);
        } else if (now >= data.regenResumeTime) {
          data.current +=
              data.max * (Settings::fPostureRegenPercentPerSecond / 100.0f) * (deltaTime / 1000.0f);
          data.current = std::clamp(data.current, 0.0f, data.max);
        }
      }
    }
  }

  // 更新不可破防状态
  {
    std::scoped_lock lock(mtx_unbreakableCache);
    for (auto it = unbreakableActors.begin(); it != unbreakableActors.end();) {
      if (it->second <= now)
        it = unbreakableActors.erase(it);
      else
        ++it;
    }
  }
}

void Posture::UpdateMaxPosture(RE::Actor* actor)
{
  if (!actor || !Settings::bUsePostureSystem)
    return;

  std::unique_lock lock(mtx_postureData);
  float maxPosture = CalculateMaxPosture(actor);
  auto& data       = GetPostureDataRef(actor);
  data.max         = maxPosture;
  if (data.current > data.max)
    data.current = data.max;
}

float Posture::GetCurrentPosture(RE::Actor* actor)
{
  return GetPostureData(actor).current;
}

float Posture::GetMaxPosture(RE::Actor* actor)
{
  return GetPostureData(actor).max;
}

Posture::PostureData Posture::GetPostureData(RE::Actor* actor)
{
  {
    std::shared_lock lock(mtx_postureData);
    if (postureMap.contains(actor))
      return postureMap[actor];
  }
  std::unique_lock lock(mtx_postureData);
  if (!postureMap.contains(actor))
    InitPosture(actor);
  return postureMap[actor];
}

void Posture::ProcessWeaponHit(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData)
{
  if (!aggressor || !victim || !Settings::bUsePostureSystem)
    return;
  if (hitData.totalDamage <= 0.0f)
    return;

  auto hitFlags     = hitData.flags;
  auto attackWeapon = hitData.weapon;

  auto bash   = hitFlags.any(RE::HitData::Flag::kBash);
  auto shield = false;
  if (auto* left = aggressor->GetEquippedObject(true); left)
    shield = left->IsArmor();

  float postureDamage = 0.0f;

  if (!attackWeapon) {
    // 在bash发生的时候，attackWeapon是空的，使用右手武器类型来判断攻击类型
    if (bash) {
      auto type = Weapon::Type::None;
      if (shield)
        type = Weapon::Type::Shield;
      else
        type = Weapon::GetActorEquipmentType(aggressor, false);
      postureDamage = Weapon::GetBasePostureDamage(type) * Settings::fBashPostureDamageMult;
    } else {
      // 武器为空且不是Bash，说明攻击来源于生物
      auto race     = Race::GetRace(aggressor);
      postureDamage = Race::GetBasePostureDamage(race);
    }
  } else {
    auto type     = Weapon::GetWeaponType(attackWeapon);
    postureDamage = Weapon::GetBasePostureDamage(type);
  }

  // Process Power Attack
  if (hitFlags.any(RE::HitData::Flag::kPowerAttack)) {
    // Process Bash Power Attack
    if (bash) {
      postureDamage *= Settings::fPowerBashPostureDamageMult;
    } else {
      postureDamage *= Settings::fPowerAttackPostureDamageMult;
    }
  } else {
    // Process Bash Attack
    if (bash) {
      postureDamage *= Settings::fBashPostureDamageMult;
    }
  }

  // 处理基于图事件的架势伤害调整
  {
    std::scoped_lock lock(mtx_damageCache);
    if (auto iter = damageCache.find(aggressor); iter != damageCache.end())
      postureDamage *= iter->second;
  }

  // Process Block
  if (hitFlags.any(RE::HitData::Flag::kBlocked)) {
    Block::ProcessPostureDamage(aggressor, victim, postureDamage);
  } else {
    DamagePostureHealth(victim, postureDamage);
  }
}
void Posture::DamagePostureHealth(RE::Actor* actor, float value, bool ignoreBreak)
{
  if (!actor)
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  if (value <= 0.0f)
    return;

  // 如果目标处于可被处决状态，则不再处理架势伤害，直接返回
  if (Execution::IsExecutable(actor))
    return;

  std::unique_lock lock(mtx_postureData);
  // 在锁中获取引用以避免同一个引用被多次获取导致的线程安全问题
  auto& postureData = GetPostureDataRef(actor);

  // 护甲值带来的架势伤害减慢
  // 这里使用指数衰减函数来计算护甲对架势伤害的减缓效果，确保在高护甲值时仍然有一定的架势伤害，但不会完全免疫
  value *= std::expf(-Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kDamageResist) *
                     Settings::fArmorPostureDamageFactor / 1000.0f);

  // 力竭状态增加架势伤害
  if (Exhausted::IsActorExhausted(actor))
    value *= Settings::fOnHitPostureDamageMultWhenExhausted;

  postureData.current -= value;
  postureData.regenResumeTime =
      Utils::GetTime<std::chrono::milliseconds>() + Settings::uPostureRegenDelay;

  // 如果此次免疫破防则不进行破防处理，即使架势值降到0也不会触发破防
  // 而是保留在0.1的微弱架势值以避免重复触发
  auto unbreakable = false;
  {
    std::scoped_lock lock(mtx_unbreakableCache);
    if (unbreakableActors.contains(actor))
      unbreakable = true;
  }

  // 如果不可破防或者忽略破防且架势值降到0或以下，则保留在0.1
  if ((unbreakable || ignoreBreak) && postureData.current <= 0.0f)
    postureData.current = 0.1f;

  // 破防处理
  if (postureData.current <= 0.0f) {
    Execution::EnterExecutable(actor);
    postureData.current = 0.5f * postureData.max;  // 进入处决状态后默认恢复到一半的最大值
  }
}

void Posture::Unbreakable(RE::Actor* actor, const std::string& payload)
{
  if (!actor)
    return;

  auto duration = Utils::toInt(payload);
  if (duration <= 0)
    return;

  std::lock_guard<std::mutex> lock(mtx_unbreakableCache);
  unbreakableActors[actor] = Utils::GetTime<std::chrono::milliseconds>() + duration;
}

void Posture::Damage(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUsePostureSystem)
    return;
  if (actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 1 && split.size() != 2)
    return;
  float damageMult   = Utils::toFloat(split[0]);
  float fallbackMult = split.size() == 2 ? Utils::toFloat(split[1]) : damageMult;
  if (damageMult < 0.0f || fallbackMult < 0.0f)
    return;
  auto subordinate =
      WeaponArt::Manager::GetPerform(actor) == WeaponArt::Manager::Perform::Subordinate;

  std::lock_guard<std::mutex> lock(mtx_damageCache);
  damageCache[actor] = subordinate ? fallbackMult : damageMult;
}

void Posture::End(RE::Actor* actor)
{
  if (!actor || !Settings::bUsePostureSystem)
    return;

  std::lock_guard<std::mutex> lock(mtx_damageCache);
  damageCache.erase(actor);
}

void Posture::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (!Settings::bUsePostureSystem)
    return;
  if (!actor)
    return;

  if (payload.starts_with("unbreakable|"))
    Unbreakable(actor, payload.substr(12));
  else if (payload.starts_with("damage|"))
    Damage(actor, payload.substr(7));
  else if (payload == "end")
    End(actor);
}