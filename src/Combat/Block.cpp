#include "Combat/Block.h"

#include "Combat/Posture.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Data/Weapon.h"
#include "Utils.h"

namespace
{
static RE::TESObjectACTI* activator = nullptr;
static RE::BGSExplosion* spark      = nullptr;
static RE::BGSExplosion* sparkFlare = nullptr;
static RE::BGSExplosion* sparkRing  = nullptr;

static RE::BGSSoundDescriptorForm* timedBlockSFX = nullptr;
static RE::BGSSoundDescriptorForm* parrySFX      = nullptr;

void PlaceBlockVFX(RE::Actor* actor, Weapon::Type type, std::uint32_t blockType)
{
  if (!actor || type == Weapon::Type::None)
    return;

  if (!activator)
    return;

  auto obj = actor->PlaceObjectAtMe(activator, false);

  if (type == Weapon::Type::Shield || type == Weapon::Type::Torch)
    obj->MoveToNode(actor, "SHIELD");
  else
    obj->MoveToNode(actor, "WEAPON");

  switch (blockType) {
  case "TimedBlock"_h:
    if (spark && sparkFlare) {
      obj->PlaceObjectAtMe(spark, false);
      obj->PlaceObjectAtMe(sparkFlare, false);
    }
    break;
  case "Parry"_h:
    if (spark && sparkFlare && sparkRing) {
      obj->PlaceObjectAtMe(spark, false);
      obj->PlaceObjectAtMe(sparkFlare, false);
      obj->PlaceObjectAtMe(sparkRing, false);
    }
    break;
  default:
    break;
  }
  obj->SetDelete(true);
}
};  // namespace

Block::Block()
{
  auto dataHandler = RE::TESDataHandler::GetSingleton();
  if (dataHandler) {
    activator     = dataHandler->LookupForm<RE::TESObjectACTI>(0x800, "RimCombat.esp");
    spark         = dataHandler->LookupForm<RE::BGSExplosion>(0x801, "RimCombat.esp");
    sparkFlare    = dataHandler->LookupForm<RE::BGSExplosion>(0x802, "RimCombat.esp");
    sparkRing     = dataHandler->LookupForm<RE::BGSExplosion>(0x803, "RimCombat.esp");
    timedBlockSFX = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x804, "RimCombat.esp");
    parrySFX      = dataHandler->LookupForm<RE::BGSSoundDescriptorForm>(0x805, "RimCombat.esp");
  }

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

  // AddTimedBlockListener([](RE::Actor* actor) {
  //   auto* vm = RE::BSScript::Internal::VirtualMachine::GetSingleton();
  //   if (!vm)
  //     return;

  //   auto args = RE::MakeFunctionArguments<RE::Actor*>(std::move(actor));
  //   vm->SendEventAll("OnRimTimedBlock", args);
  // });
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
    std::unique_lock lock(mtx_timedBlockDuration);
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
      Stagger::StaggerStart(aggressor);

      auto type = Weapon::GetBlockType(victim);

      PlaceBlockVFX(victim, type, "Parry"_h);
      Utils::PlaySFX(victim, parrySFX, victim->GetPosition());

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
  if (!blocked)
    return;

  // 首次限时格挡检测
  std::uint64_t startTime = 0;
  {
    std::scoped_lock lock(mtx_blockStart);

    // 必须存在格挡开始的记录
    if (blockStartTimes.contains(victim)) {
      startTime = blockStartTimes[victim];
      if (Settings::bTimedBlockEnabled && now - startTime < Settings::uTimedBlockLimit) {
        std::unique_lock lock(mtx_timedBlockDuration);
        timedBlockEndTimes[victim] = now + Settings::uTimedBlockDuration;
        timedBlock                 = true;
      }

      // 每次格挡只能触发一次限时格挡，所以这里直接删除记录
      blockStartTimes.erase(victim);
    }
  }
  // 非首次限时格挡时受击
  {
    std::unique_lock lock(mtx_timedBlockDuration);
    if (!startTime && timedBlockEndTimes.contains(victim)) {
      timedBlockEndTimes[victim] = now + Settings::uTimedBlockDuration;
      timedBlock                 = true;
    }
  }
  // 限时格挡事件分发

  // auto message = std::format("{} 限时格挡", victim->GetDisplayFullName());
  // RE::SendHUDMessage::ShowHUDMessage(message.data());

  float stamina      = victim->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina);
  float maxStamina   = Utils::GetCurrentMaxActorValue(victim, RE::ActorValue::kStamina);
  auto type          = Weapon::GetBlockType(victim);
  auto blockStrength = Weapon::GetBlockStrength(type);

  if (timedBlock) {
    blockStrength *= Settings::fTimedBlockBlockStrengthMult;
    PlaceBlockVFX(victim, type, "TimedBlock"_h);
    Utils::PlaySFX(victim, timedBlockSFX, victim->GetPosition());
  }

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

  // 如果耐力消耗超过当前耐力，则触发格挡破防
  if (staminaConsume > stamina)
    Stagger::SetStaggerLevel(victim, Stagger::Level::GuardBreak);

  // 应用格挡耐力增益
  damage -= (staminaConsume + Settings::fBlockStaminaBonus) / staminaPerDamage;
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

  auto now = Utils::GetTime<std::chrono::milliseconds>();

  auto endTime = Utils::toInt(split[0]);
  if (!endTime)
    return;
  if (endTime.value() <= 0)
    return;
  data.endTime = now + endTime.value();

  auto level = Utils::toInt(split[1]);
  if (!level)
    return;

  data.level = static_cast<Stagger::Level>(level.value());
  if (data.level == Stagger::Level::None || data.level > Stagger::Level::Large)
    return;

  data.autoAttack    = split[2] == "true";
  data.isPowerAttack = split[3].starts_with("p");

  auto nextAttack = Utils::toInt(split[3].substr(1));
  if (!nextAttack)
    return;
  data.nextAttack = static_cast<std::uint8_t>(nextAttack.value());
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
  if (!duration)
    return;
  if (duration <= 0)
    return;

  std::lock_guard lock(mtx_parry);
  parryEndTimes[actor] = Utils::GetTime<std::chrono::milliseconds>() + duration.value();
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