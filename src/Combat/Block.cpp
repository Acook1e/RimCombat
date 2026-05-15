#include "Combat/Block.h"

#include "Combat/Posture.h"
#include "Combat/Weapon.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Utils.h"

Block::Block()
{
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    {
      std::lock_guard lock(mtx_blockStart);
      blockStartTimes.clear();
    }
    {
      std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
      timedBlockDurationStartTimes.clear();
    }
  });
}

void Block::Update()
{
  if (!Settings::bUseBlockSystem)
    return;

  auto now = Utils::GetTime<std::chrono::milliseconds>();

  // 定期清理过期的格挡时间记录，避免map无限增长
  {
    std::lock_guard lock(mtx_blockStart);
    for (auto it = blockStartTimes.begin(); it != blockStartTimes.end();) {
      if (now - it->second > Settings::uTimedBlockLimit)
        it = blockStartTimes.erase(it);
      else
        ++it;
    }
  }

  // 计时限时格挡的持续时间，超过持续时间的记录会被移除
  {
    std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
    for (auto it = timedBlockDurationStartTimes.begin();
         it != timedBlockDurationStartTimes.end();) {
      if (now - it->second > Settings::uTimedBlockDuration)
        it = timedBlockDurationStartTimes.erase(it);
      else
        ++it;
    }
  }
}

void Block::StartBlock(RE::Actor* actor)
{
  if (!actor || !Settings::bUseBlockSystem)
    return;
  std::lock_guard lock(mtx_blockStart);
  blockStartTimes[actor] = Utils::GetTime<std::chrono::milliseconds>();
}

void Block::EndBlock(RE::Actor* actor)
{
  if (!actor || !Settings::bUseBlockSystem)
    return;
  {
    std::scoped_lock lock(mtx_blockStart);
    blockStartTimes.erase(actor);
  }
  {
    std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
    timedBlockDurationStartTimes.erase(actor);
  }
}

void Block::ProcessBlock(RE::Actor* actor)
{
  if (!actor || !Settings::bUseBlockSystem || !Settings::bTimedBlockEnabled)
    return;

  {
    std::shared_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
    if (timedBlockDurationStartTimes.contains(actor))
      return;
  }

  std::uint64_t startTime = 0;
  {
    std::scoped_lock lock(mtx_blockStart);
    // 如果没有格挡记录，说明不是限时格挡
    if (!blockStartTimes.contains(actor))
      return;

    // 每次格挡只能触发一次限时格挡，所以这里直接删除记录
    startTime = blockStartTimes[actor];
    blockStartTimes.erase(actor);
  }

  auto now = Utils::GetTime<std::chrono::milliseconds>();
  if (now - startTime < Settings::uTimedBlockLimit) {
    {
      std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
      timedBlockDurationStartTimes[actor] = now;
    }
    std::vector<std::function<void(RE::Actor*)>> callbacks;
    {
      std::lock_guard<std::mutex> lock(mtx_timedBlockCallback);
      callbacks = timedBlockCallbacks;
    }
    for (auto& callback : callbacks)
      callback(actor);
  }
}

void Block::ProcessDamage(RE::Actor* victim, RE::HitData& hitData)
{
  if (!victim || !Settings::bUseBlockSystem)
    return;
  if (victim->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode())
    return;

  float stamina      = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
  float maxStamina   = Utils::GetCurrentMaxActorValue(victim, RE::ActorValue::kStamina);
  auto type          = Weapon::GetBlockType(victim);
  auto blockStrength = Weapon::GetBlockStrength(type);
  bool timedBlock    = IsTimedBlock(victim);

  if (timedBlock)
    blockStrength *= Settings::fTimedBlockBlockStrengthMult;

  if (blockStrength <= 0.0f)
    return;

  float maxDamageConsumePercent = blockStrength / (blockStrength + 15.0f);
  float staminaPerDamage        = 30 / blockStrength;

  float damage = hitData.totalDamage;
  if (damage <= 0)
    return;

  float staminaConsume = damage * maxDamageConsumePercent * staminaPerDamage;

  // 超出单次耐力消耗上限的部分不再增加耐力消耗
  if (staminaConsume > maxStamina * Settings::fBlockMaxStaminaConsumePercent)
    staminaConsume = maxStamina * Settings::fBlockMaxStaminaConsumePercent;

  // 如果耐力不足以完全格挡伤害，则至少消耗一定的耐力
  if (staminaConsume > stamina + Settings::fBlockMinStaminaConsume)
    staminaConsume = stamina + Settings::fBlockMinStaminaConsume;

  damage -= staminaConsume / staminaPerDamage;
  if (damage < 0)
    damage = 0;

  // 写回修改伤害数据
  hitData.totalDamage = damage;
  // 归零0格挡率，避免二次消耗耐力
  hitData.percentBlocked = 0.0f;
  victim->AsActorValueOwner()->DamageActorValue(RE::ActorValue::kStamina, staminaConsume);
}

void Block::ProcessPostureDamage(RE::Actor* aggressor, RE::Actor* victim, float postureDamage)
{
  if (!victim || !Settings::bUseBlockSystem)
    return;

  auto type          = Weapon::GetBlockType(victim);
  auto blockStrength = Weapon::GetBlockStrength(type);
  bool timedBlock    = IsTimedBlock(victim);

  if (timedBlock)
    blockStrength *= Settings::fTimedBlockBlockStrengthMult;

  if (blockStrength <= 0.0f)
    return;

  // 格挡时受击者的架势伤害乘数，乘以基础架势伤害
  float postureDamageMult = 25.0 / (blockStrength + 25.0f);
  // 格挡时返回给攻击者的架势伤害乘数
  float postureDamageReflectMult = blockStrength * 0.002f;

  Posture::DamagePostureValue(aggressor, postureDamage * postureDamageReflectMult, true);
  Posture::DamagePostureValue(victim, postureDamage * postureDamageMult,
                              timedBlock && Settings::bTimedBlockNeverPostureBreak);
}

bool Block::IsTimedBlock(RE::Actor* actor)
{
  if (!actor || !Settings::bUseBlockSystem || !Settings::bTimedBlockEnabled)
    return false;

  std::shared_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
  auto it = timedBlockDurationStartTimes.find(actor);
  if (it == timedBlockDurationStartTimes.end())
    return false;
  auto now = Utils::GetTime<std::chrono::milliseconds>();
  return (now - it->second) <= Settings::uTimedBlockDuration;
}

void Block::AddTimedBlockListener(std::function<void(RE::Actor*)> callback)
{
  std::lock_guard lock(mtx_timedBlockCallback);
  timedBlockCallbacks.push_back(callback);
}