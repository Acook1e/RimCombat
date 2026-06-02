#include "Combat/Block.h"

#include "Combat/Posture.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"
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
      timedBlockEndTimes.clear();
    }
    {
      std::lock_guard lock(mtx_gpData);
      gpData.clear();
    }
    {
      std::lock_guard lock(mtx_parry);
      parryEndTimes.clear();
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

  // 计时限时格挡的截止时间，超过截止时间的记录会被移除
  {
    std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
    for (auto it = timedBlockEndTimes.begin(); it != timedBlockEndTimes.end();) {
      if (now > it->second)
        it = timedBlockEndTimes.erase(it);
      else
        ++it;
    }
  }

  // GP状态的管理，超过GP窗口的记录会被移除
  {
    std::lock_guard lock(mtx_gpData);
    for (auto it = gpData.begin(); it != gpData.end();) {
      if (now > it->second.endTime)
        it = gpData.erase(it);
      else
        ++it;
    }
  }

  // 弹反状态的管理，超过弹反窗口的记录会被移除
  {
    std::lock_guard lock(mtx_parry);
    for (auto it = parryEndTimes.begin(); it != parryEndTimes.end();) {
      if (now > it->second)
        it = parryEndTimes.erase(it);
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
    timedBlockEndTimes.erase(actor);
  }
}

void Block::ProcessBlock(RE::Actor* aggressor, RE::Actor* victim, RE::HitData& hitData)
{
  if (!victim || !Settings::bUseBlockSystem)
    return;

  bool blocked = hitData.flags.any(RE::HitData::Flag::kBlocked);

  // 弹反拥有最高的优先级，在任何格挡类型之前处理
  // 弹反成功直接将伤害清零并给予攻击者GuardBreak级别的硬直
  // 并且直接返回，不再处理后续的格挡和韧性等系统，以避免多重干预导致的平衡性问题
  {
    std::lock_guard<std::mutex> lock(mtx_parry);
    if (parryEndTimes.contains(victim)) {
      hitData.totalDamage = 0.0f;
      Stagger::SetStaggerLevel(aggressor, Stagger::Level::GuardBreak);
      Stagger::SetStaggerMagnitude(aggressor, Stagger::Level::GuardBreak);
      Stagger::StaggerStart(aggressor);
      return;
    }
  }

  // GP为第二优先级
  bool timedBlock = false;
  auto now        = Utils::GetTime<std::chrono::milliseconds>();
  {
    std::lock_guard<std::mutex> lock(mtx_gpData);
    if (gpData.contains(victim)) {
      // GP成功后将本次命中视为格挡
      blocked = true;
      hitData.flags.set(true, RE::HitData::Flag::kBlocked);
      {
        // 给予极短的限时格挡，触发后续的格挡强化和反击效果
        std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
        timedBlockEndTimes[victim] = now + 10;
        timedBlock                 = true;
      }
      auto& data = gpData[victim];
      // 给予攻击者硬直
      Stagger::SetStaggerLevel(aggressor, data.level);
      Stagger::SetStaggerMagnitude(aggressor, data.level);
      Stagger::StaggerStart(aggressor);
      // 设置下一次攻击的段数
      if (data.nextAttack > 0) {
        if (data.isPowerAttack)
          victim->SetGraphVariableInt("MCO_nextpowerattack", data.nextAttack);
        else
          victim->SetGraphVariableInt("MCO_nextattack", data.nextAttack);
      }
      // 决定是否自动反击
      if (data.autoAttack) {
        if (data.isPowerAttack)
          victim->NotifyAnimationGraph("attackPowerStartInPlace");
        else
          victim->NotifyAnimationGraph("attackStart");
      }
      // 移除GP数据，避免重复触发
      gpData.erase(victim);
    }
  }

  // 普通格挡和限时格挡必须在格挡中
  std::uint64_t startTime = 0;
  if (blocked) {
    std::scoped_lock lock(mtx_blockStart);
    // 如果没有格挡记录，说明不是限时格挡
    if (!blockStartTimes.contains(victim))
      return;

    // 每次格挡只能触发一次限时格挡，所以这里直接删除记录
    startTime = blockStartTimes[victim];
    blockStartTimes.erase(victim);
  }
  if (blocked && Settings::bTimedBlockEnabled && now - startTime < Settings::uTimedBlockLimit) {
    {
      // 直接覆写或插入限时格挡计时记录，无需检查是否存在
      // ProcessBlock每次被调用都视为一次新的限时格挡触发，因此直接重置计时
      std::unique_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
      timedBlockEndTimes[victim] = now + Settings::uTimedBlockDuration;
      timedBlock                 = true;
    }
    std::vector<std::function<void(RE::Actor*)>> callbacks;
    {
      std::lock_guard<std::mutex> lock(mtx_timedBlockCallback);
      callbacks = timedBlockCallbacks;
    }
    for (auto& callback : callbacks)
      callback(victim);
  }

  if (!blocked)
    return;

  float stamina      = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
  float maxStamina   = Utils::GetCurrentMaxActorValue(victim, RE::ActorValue::kStamina);
  auto type          = Weapon::GetBlockType(victim);
  auto blockStrength = Weapon::GetBlockStrength(type);

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

  // 如果耐力消耗超过当前耐力，则触发格挡破防
  if (staminaConsume > stamina)
    Stagger::SetStaggerLevel(victim, Stagger::Level::GuardBreak);

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
  bool timedBlock    = IsTimedBlocking(victim);

  if (timedBlock)
    blockStrength *= Settings::fTimedBlockBlockStrengthMult;

  if (blockStrength <= 0.0f)
    return;

  // 格挡时受击者的架势伤害乘数，乘以基础架势伤害
  float postureDamageMult = 25.0 / (blockStrength + 25.0f);
  // 格挡时返回给攻击者的架势伤害乘数
  float postureDamageReflectMult = blockStrength * 0.002f;

  Posture::DamagePostureHealth(aggressor, postureDamage * postureDamageReflectMult, true);
  Posture::DamagePostureHealth(victim, postureDamage * postureDamageMult,
                               timedBlock && Settings::bTimedBlockNeverPostureBreak);
}

bool Block::IsBlocking(RE::Actor* actor)
{
  if (!actor || !Settings::bUseBlockSystem)
    return false;

  auto block = false;
  {
    std::lock_guard lock(mtx_blockStart);
    block = blockStartTimes.contains(actor);
  }

  auto timedBlock = false;
  {
    std::shared_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
    timedBlock = timedBlockEndTimes.contains(actor);
  }
  return block || timedBlock || actor->IsBlocking();
}

bool Block::IsTimedBlocking(RE::Actor* actor)
{
  if (!actor || !Settings::bUseBlockSystem || !Settings::bTimedBlockEnabled)
    return false;

  std::shared_lock<std::shared_mutex> lock(mtx_timedBlockDuration);
  auto it = timedBlockEndTimes.find(actor);
  if (it == timedBlockEndTimes.end())
    return false;
  auto now = Utils::GetTime<std::chrono::milliseconds>();
  return now < it->second;
}

void Block::AddTimedBlockListener(std::function<void(RE::Actor*)> callback)
{
  std::lock_guard lock(mtx_timedBlockCallback);
  timedBlockCallbacks.push_back(callback);
}

void Block::GP(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseBlockSystem)
    return;

  GPData data;
  auto split = Utils::split(payload, '|');

  if (split.size() != 4) {
    logger::warn("Invalid GP payload: {}", payload);
    return;
  }

  auto now     = Utils::GetTime<std::chrono::milliseconds>();
  auto endTime = Utils::toInt(split[0]);
  if (endTime <= 0) {
    logger::warn("Invalid GP duration: {}", split[0]);
    return;
  }
  data.endTime = now + endTime;

  data.level = static_cast<Stagger::Level>(Utils::toInt(split[1]));
  if (data.level == Stagger::Level::None || data.level > Stagger::Level::Large) {
    logger::warn("Invalid GP stagger level: {}", split[1]);
    return;
  }

  data.autoAttack    = split[2] == "true";
  data.isPowerAttack = split[3].starts_with("p");
  data.nextAttack    = static_cast<std::uint8_t>(Utils::toInt(split[3].substr(1)));
  if (data.nextAttack == 0)
    data.autoAttack = false;

  {
    std::lock_guard lock(mtx_gpData);
    gpData[actor] = std::move(data);
  }
}

void Block::Parry(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseBlockSystem)
    return;

  auto duration = Utils::toInt(payload);
  if (duration <= 0) {
    logger::warn("Invalid Parry duration: {}", payload);
    return;
  }

  std::lock_guard lock(mtx_parry);
  parryEndTimes[actor] = Utils::GetTime<std::chrono::milliseconds>() + duration;
}

void Block::ParsePayload(RE::Actor* actor, const std::string& payload)
{
  if (!actor || !Settings::bUseBlockSystem)
    return;

  if (payload.starts_with("gp|"))
    GP(actor, payload.substr(3));
  else if (payload.starts_with("parry|"))
    Parry(actor, payload.substr(6));
}