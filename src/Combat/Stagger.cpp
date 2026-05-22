#include "Combat/Stagger.h"

#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"

using Level = Stagger::Level;

Stagger::Stagger()
{
  constexpr std::string_view MSLSettingsFile = "Data/SKSE/Plugins/ModernStaggerLock.ini";

  std::error_code ec;
  if (!std::filesystem::exists(MSLSettingsFile, ec)) {
    logger::warn("Stagger: Modern Stagger Lock settings file not found: {}", MSLSettingsFile);
    return;
  }

  auto path = std::filesystem::path(MSLSettingsFile);

  std::ifstream ifs(path);
  if (!ifs.is_open()) {
    logger::warn("Stagger: failed to open Modern Stagger Lock settings file: {}", MSLSettingsFile);
    return;
  }

  std::string line;
  while (std::getline(ifs, line)) {
    // 解析类似于 "Small=0.25" 的行
    auto pos = line.find('=');
    if (pos == std::string::npos)
      continue;

    auto split = Utils::split(line, '=');
    if (split.size() != 2)
      continue;

    auto key      = split[0];
    auto valueStr = split[1];

    // key 转为小写
    std::transform(key.begin(), key.end(), key.begin(), ::tolower);
    // valueStr 转去除空格
    valueStr.erase(std::remove_if(valueStr.begin(), valueStr.end(), ::isspace), valueStr.end());

    // largest可能包含largest和large两个关键词
    // 因此判断LargeStagger和LargestStagger的关键词，确保正确匹配
    if (key.find("small") != std::string::npos) {
      staggerMagnitudeMap[Level::Small] = Utils::toFloat(valueStr);
    } else if (key.find("medium") != std::string::npos) {
      staggerMagnitudeMap[Level::Medium] = Utils::toFloat(valueStr);
    } else if (key.find("largestagger") != std::string::npos) {
      staggerMagnitudeMap[Level::Large] = Utils::toFloat(valueStr);
    } else if (key.find("largeststagger") != std::string::npos) {
      staggerMagnitudeMap[Level::Largest] = Utils::toFloat(valueStr);
    }
  }

  logger::info("Stagger: Modern Stagger Lock stagger magnitude thresholds loaded");

  // 使用序列化系统重置缓存
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    {
      std::scoped_lock lock(mtx_targetCache);
      staggerLevelOnHit.clear();
    }
    {
      std::scoped_lock lock(mtx_immuneCache);
      immuneActors.clear();
    }
  });
}

void Stagger::Update()
{
  auto now = Utils::GetTime<std::chrono::milliseconds>();

  // 更新免疫缓存，移除过期的免疫状态
  {
    std::scoped_lock lock(mtx_immuneCache);
    for (auto it = immuneActors.begin(); it != immuneActors.end();) {
      if (it->second.second <= now)
        it = immuneActors.erase(it);
      else
        ++it;
    }
  }

  // 更新恢复时间缓存
  {
    std::scoped_lock lock(mtx_recoverTime);
    for (auto it = staggerRecoverTime.begin(); it != staggerRecoverTime.end();) {
      auto victim      = it->first;
      auto recoverTime = it->second.second;
      if (recoverTime <= now) {
        if (victim)
          victim->SetGraphVariableBool(STAGGER_RECOVERABLE, true);
        it = staggerRecoverTime.erase(it);
      } else
        ++it;
    }
  }
}

float Stagger::LevelToMagnitude(Level level)
{
  if (level >= Level::Largest)
    return staggerMagnitudeMap[Level::Largest];

  if (auto it = staggerMagnitudeMap.find(level); it != staggerMagnitudeMap.end())
    return it->second;

  return 0.0f;
}

Level Stagger::MagnitudeToLevel(float magnitude)
{
  if (magnitude >= LevelToMagnitude(Level::Largest))
    return Level::Largest;
  else if (magnitude >= LevelToMagnitude(Level::Large))
    return Level::Large;
  else if (magnitude >= LevelToMagnitude(Level::Medium))
    return Level::Medium;
  else if (magnitude >= LevelToMagnitude(Level::Small))
    return Level::Small;
  return Level::None;
}

float Stagger::GetStaggerMagnitude(RE::Actor* actor)
{
  if (!actor)
    return 0.0f;

  float staggerMagnitude = 0.0f;
  if (!actor->GetGraphVariableFloat("staggerMagnitude", staggerMagnitude))
    return 0.0f;

  return staggerMagnitude;
}

void Stagger::SetStaggerMagnitude(RE::Actor* actor, Level level)
{
  if (!actor)
    return;

  float magnitude = LevelToMagnitude(level);
  actor->SetGraphVariableFloat("staggerMagnitude", magnitude);
}

Level Stagger::GetStaggerLevel(RE::Actor* actor)
{
  if (!actor)
    return Level::None;

  std::int32_t level = 0;
  if (!actor->GetGraphVariableInt(STAGGER_LEVEL, level))
    return Level::None;

  if (level < static_cast<std::int32_t>(Level::None) ||
      level >= static_cast<std::int32_t>(Level::Total))
    return Level::None;

  return static_cast<Level>(level);
}

void Stagger::SetStaggerLevel(RE::Actor* actor, Level level)
{
  if (!actor)
    return;

  actor->SetGraphVariableInt(STAGGER_LEVEL, static_cast<std::int8_t>(level));
}

bool Stagger::IsImmune(RE::Actor* actor)
{
  if (!actor)
    return false;

  auto immune    = GetImmuneLevel(actor);
  auto magnitude = GetStaggerMagnitude(actor);
  auto stagger   = MagnitudeToLevel(magnitude);

  if (immune >= stagger)
    return true;

  return false;
}

Level Stagger::GetImmuneLevel(RE::Actor* actor)
{
  if (!actor)
    return Level::None;

  std::lock_guard lock(mtx_immuneCache);
  if (auto it = immuneActors.find(actor); it != immuneActors.end())
    return it->second.first;

  return Level::None;
}

void Stagger::ProcessStagger(RE::Actor* aggressor, RE::Actor* victim)
{
  if (!aggressor || !victim || !Settings::bUseStaggerSystem)
    return;

  Level currentLevel = GetStaggerLevel(victim);

  // 处理外源硬直
  // 不清除Map中的数据，直到EndTarget事件，以确保在攻击过程中持续生效
  Level targetLevel = Level::None;
  {
    std::lock_guard lock(mtx_targetCache);
    if (auto it = staggerLevelOnHit.find(aggressor); it != staggerLevelOnHit.end())
      targetLevel = it->second;
  }

  if (targetLevel != Level::None || currentLevel < targetLevel)
    currentLevel = targetLevel;

  SetStaggerLevel(victim, currentLevel);
  SetStaggerMagnitude(victim, currentLevel);

  if (currentLevel == Level::None)
    return;

  std::uint64_t recoverTime = 0;
  switch (currentLevel) {
  case Level::Small:
    recoverTime = Settings::uStaggerRecoveryTimeSmall;
    break;
  case Level::Medium:
    recoverTime = Settings::uStaggerRecoveryTimeMedium;
    break;
  case Level::Large:
    recoverTime = Settings::uStaggerRecoveryTimeLarge;
    break;
  default:
    break;
  }

  if (recoverTime > 0) {
    auto now = Utils::GetTime<std::chrono::milliseconds>();
    std::scoped_lock<std::mutex> lock(mtx_recoverTime);
    if (!staggerRecoverTime.contains(victim))
      staggerRecoverTime[victim] = {currentLevel, now + recoverTime};
    else {
      auto [lastLevel, lastTime] = staggerRecoverTime[victim];

      // 如果在大硬直期间受到小硬直，则忽略这次硬直
      if (lastLevel < currentLevel)
        return;

      auto delta = lastTime - now;
      // TODO：找一个比相加乘以0.2更合理的算法
      recoverTime                = (delta + recoverTime) * 0.2f;
      staggerRecoverTime[victim] = {lastLevel, now + recoverTime};
    }
  }

  if (victim->IsStaggering())
    victim->NotifyAnimationGraph("staggerStop");
  victim->SetGraphVariableBool(STAGGER_RECOVERABLE, false);
  victim->NotifyAnimationGraph("staggerStart");
  SetStaggerLevel(victim, Level::None);
}

void Stagger::TargetSet(RE::Actor* actor, const std::string& payload)
{
  if (!actor)
    return;

  // 如果是战技的属性，那仅在Eligible状态生效，Subordinate状态不生效
  if (WeaponArt::Manager::GetPerform(actor) == WeaponArt::Manager::Perform::Subordinate)
    return;

  auto level = static_cast<Level>(Utils::toInt(payload));
  if (level == Level::None)
    return;
  std::lock_guard lock(mtx_targetCache);
  staggerLevelOnHit[actor] = level;
}

void Stagger::TargetEnd(RE::Actor* actor)
{
  if (!actor)
    return;

  std::lock_guard lock(mtx_targetCache);
  staggerLevelOnHit.erase(actor);
}

void Stagger::Immune(RE::Actor* actor, const std::string& payload)
{
  if (!actor)
    return;

  // 如果是战技的属性，那仅在Eligible状态生效，Subordinate状态不生效
  if (WeaponArt::Manager::GetPerform(actor) == WeaponArt::Manager::Perform::Subordinate)
    return;

  auto split = Utils::split(payload, '|');
  if (split.size() != 2)
    return;

  auto immuneLevel = static_cast<Level>(Utils::toInt(split[0]));
  auto duration    = Utils::toInt(split[1]);

  if (immuneLevel == Level::None || immuneLevel > Level::Largest || duration <= 0)
    return;

  std::lock_guard lock(mtx_immuneCache);
  immuneActors[actor] = {immuneLevel, Utils::GetTime<std::chrono::milliseconds>() + duration};
}

void Stagger::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload.starts_with("targetset|"))
    TargetSet(actor, payload.substr(10));
  else if (payload == "targetend")
    TargetEnd(actor);
  else if (payload.starts_with("immune|"))
    Immune(actor, payload.substr(7));
}