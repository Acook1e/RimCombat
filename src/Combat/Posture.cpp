#include "Combat/Posture.h"

#include "Combat/Block.h"
#include "Combat/Exhausted.h"
#include "Core/Serialization.h"
#include "Utils.h"

Posture::Posture()
{
  Serialization::RegisterSaveCallback(
      posture, [](SKSE::SerializationInterface* serial) {
        std::scoped_lock lock(mtx);
        // 将FormID转换为持久化格式
        // 并自动去除非法或未找到的FormID
        std::unordered_map<std::uint64_t, PostureData> persistMap;
        for (const auto& [formID, data] : postureMap) {
          auto persist = Serialization::ToPersistForm(formID);
          if (persist)
            persistMap[persist] = std::move(data);
        }

        std::uint32_t count = static_cast<std::uint32_t>(persistMap.size());
        serial->WriteRecordData(&count, sizeof(count));
        for (const auto& [persist, data] : persistMap) {
          serial->WriteRecordData(&persist, sizeof(persist));
          serial->WriteRecordData(&data, sizeof(data));
        }
      });

  Serialization::RegisterLoadCallback(
      posture, [](SKSE::SerializationInterface* serial) {
        std::scoped_lock lock(mtx);
        postureMap.clear();
        runtimePostureMap.clear();

        std::uint32_t count;
        if (serial->ReadRecordData(&count, sizeof(count))) {
          for (std::uint32_t i = 0; i < count; ++i) {
            std::uint64_t persist;
            PostureData data;
            if (serial->ReadRecordData(&persist, sizeof(persist)) &&
                serial->ReadRecordData(&data, sizeof(data))) {
              auto formID = Serialization::ToForm(persist);
              if (formID)
                postureMap[formID] = std::move(data);
            }
          }
        }
      });

  Serialization::RegisterRevertCallback(posture,
                                        [](SKSE::SerializationInterface*) {
                                          std::scoped_lock lock(mtx);
                                          postureMap.clear();
                                          runtimePostureMap.clear();
                                        });
}

float Posture::InitPosture(RE::Actor* actor)
{
  // 仅在获取值时初始化架势数据，避免不必要的初始化开销
  // 因此可以无锁

  float maxPosture = Settings::fMaxPostureBase;
  maxPosture += Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kHealth) *
                Settings::fMaxPostureHealthMult;
  if (actor->GetActorBase()->IsUnique())
    postureMap[actor->GetFormID()] = {maxPosture, maxPosture};
  else
    runtimePostureMap[actor] = {maxPosture, maxPosture};
  return maxPosture;
}

void Posture::ReCalculateMaxPosture(RE::Actor* actor)
{
  std::scoped_lock lock(mtx);
  float maxPosture = Settings::fMaxPostureBase;
  maxPosture += Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kHealth) *
                Settings::fMaxPostureHealthMult;
  if (actor->GetActorBase()->IsUnique()) {
    if (postureMap.contains(actor->GetFormID()))
      postureMap[actor->GetFormID()].max = maxPosture;
    else
      postureMap[actor->GetFormID()] = {maxPosture, maxPosture};
  } else {
    if (runtimePostureMap.contains(actor))
      runtimePostureMap[actor].max = maxPosture;
    else
      runtimePostureMap[actor] = {maxPosture, maxPosture};
  }
}

float Posture::GetCurrentPosture(RE::Actor* actor)
{
  std::scoped_lock lock(mtx);
  if (actor->GetActorBase()->IsUnique()) {
    if (postureMap.contains(actor->GetFormID()))
      return postureMap[actor->GetFormID()].current;
  } else {
    if (runtimePostureMap.contains(actor))
      return runtimePostureMap[actor].current;
  }
  return InitPosture(actor);
}

float Posture::GetMaxPosture(RE::Actor* actor)
{
  std::scoped_lock lock(mtx);
  if (actor->GetActorBase()->IsUnique()) {
    if (postureMap.contains(actor->GetFormID()))
      return postureMap[actor->GetFormID()].max;
  } else {
    if (runtimePostureMap.contains(actor))
      return runtimePostureMap[actor].max;
  }
  return InitPosture(actor);
}

Posture::PostureData& Posture::GetPostureData(RE::Actor* actor)
{
  std::scoped_lock lock(mtx);
  if (actor->GetActorBase()->IsUnique()) {
    if (postureMap.contains(actor->GetFormID()))
      return postureMap[actor->GetFormID()];
  } else {
    if (runtimePostureMap.contains(actor))
      return runtimePostureMap[actor];
  }
  InitPosture(actor);
  if (actor->GetActorBase()->IsUnique())
    return postureMap[actor->GetFormID()];
  else
    return runtimePostureMap[actor];
}

void Posture::ProcessMeleeHit(RE::Actor* aggressor, RE::Actor* victim,
                              RE::HitData& hitData, bool isTimedBlock)
{
  auto hitFlags     = hitData.flags;
  auto attackWeapon = hitData.weapon;

  float postureDamage = 0.0f;
  if (!attackWeapon) {
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      postureDamage = Settings::fBashPostureDamageBase;
    } else {
      logger::warn("Posture::ProcessMeleeHit: What are you doing? {}",
                   hitFlags.underlying());
    }
  } else {
    switch (attackWeapon->GetWeaponType()) {
    case RE::WEAPON_TYPE::kHandToHandMelee:
      postureDamage = Settings::fNormalAttackPostureDamage_Fist;
      break;
    case RE::WEAPON_TYPE::kOneHandDagger:
      postureDamage = Settings::fNormalAttackPostureDamage_Dagger;
      break;
    case RE::WEAPON_TYPE::kOneHandSword:
      postureDamage = Settings::fNormalAttackPostureDamage_Sword;
      break;
    case RE::WEAPON_TYPE::kOneHandAxe:
      postureDamage = Settings::fNormalAttackPostureDamage_Axe;
      break;
    case RE::WEAPON_TYPE::kOneHandMace:
      postureDamage = Settings::fNormalAttackPostureDamage_Mace;
      break;
    case RE::WEAPON_TYPE::kTwoHandSword:
      postureDamage = Settings::fNormalAttackPostureDamage_GreatSword;
      break;
    case RE::WEAPON_TYPE::kTwoHandAxe:
      postureDamage = Settings::fNormalAttackPostureDamage_GreatAxe;
      break;
    case RE::WEAPON_TYPE::kBow:
    case RE::WEAPON_TYPE::kStaff:
    case RE::WEAPON_TYPE::kCrossbow:
      // TODO: stamina consumption for ranged weapons
      // need a calculation for posture damage based on stamina consumption
      // and distance
      break;
    default:
      logger::warn("Posture::ProcessMeleeHit: Unsupported weapon type: {}",
                   static_cast<int>(attackWeapon->GetWeaponType()));
    }
  }

  // Process Power Attack
  if (hitFlags.any(RE::HitData::Flag::kPowerAttack)) {
    // Process Bash Power Attack
    if (hitFlags.any(RE::HitData::Flag::kBash)) {
      postureDamage *= Settings::fPowerBashPostureDamageMult;
    } else {
      postureDamage *= Settings::fPowerAttackPostureDamageMult;
    }
  }

  // Process Block
  if (hitFlags.any(RE::HitData::Flag::kBlocked)) {
    ModPostureValue(aggressor,
                    -postureDamage * Settings::fBlockPostureDamageToAttacker);
    if (isTimedBlock)
      ModPostureValue(victim,
                      -postureDamage * Settings::fTimedBlockPostureDamageMult,
                      true);
    else
      ModPostureValue(victim,
                      -postureDamage * Settings::fBlockPostureDamageMult);
  } else {
    ModPostureValue(victim, -postureDamage);
  }
}
void Posture::ModPostureValue(RE::Actor* actor, float value, bool ignoreBreak)
{
  if (value == 0.0f)
    return;

  auto& postureData = GetPostureData(actor);

  std::scoped_lock lock(mtx);
  // 加值处理
  if (value < 0.0f) {
    // 护甲减伤
    value *= std::expf(
        -Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kDamageResist) *
        Settings::fArmorPostureDamageFactor / 1000.0f);
    // 力竭状态增加架势伤害
    if (Settings::bEnableExhausted && Exhausted::IsActorExhausted(actor)) {
      value *= Settings::fExhaustedPostureDamageMult;
      // 如果设置了被击打时退出力竭状态，则在此处退出
      if (Settings::bExitExhaustedOnHit)
        Exhausted::ExitExhausted(actor);
    }
  } else {
    if (postureData.current == postureData.max)
      return;
  }

  postureData.current += value;

  // 加值后处理

  // 如果此次免疫破防则不进行破防处理，即使架势值降到0也不会触发破防
  // 而是保留在0.1的微弱架势值以避免重复触发
  if (ignoreBreak && postureData.current <= 0.0f)
    postureData.current = 0.1f;

  // 破防处理
  if (postureData.current <= 0.0f) {
    PostureBreak(actor);
    postureData.current = postureData.max;
  } else if (postureData.current > postureData.max) {
    // 架势值异常处理
    postureData.current = postureData.max;
  }
}

void Posture::PostureBreak(RE::Actor* actor)
{
  if (!actor)
    return;
  SKSE::GetTaskInterface()->AddTask([actor]() {
    actor->SetGraphVariableFloat("staggerMagnitude", 1.0f);
    actor->NotifyAnimationGraph("staggerStart");
  });
}
