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

  logger::info("Stagger: Modern Stagger Lock stagger magnitude thresholds loaded:");

  // 使用序列化系统重置缓存
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    {
      std::scoped_lock lock(mtx_targetCache);
      setTargetLevelMap.clear();
      modifyTargetLevelMap.clear();
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
  std::scoped_lock lock(mtx_immuneCache);
  for (auto it = immuneActors.begin(); it != immuneActors.end();) {
    if (it->second <= now) {
      SetImmuneLevel(it->first, Level::None);
      it = immuneActors.erase(it);
    } else
      ++it;
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

  logger::info("Actor {} is set to Level {}", actor->GetDisplayFullName(),
               magic_enum::enum_name(level));

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

  std::int32_t immune = 0;
  if (!actor->GetGraphVariableInt(STAGGER_IMMUNE, immune))
    return Level::None;

  if (immune < static_cast<std::int32_t>(Level::None) ||
      immune >= static_cast<std::int32_t>(Level::Total))
    return Level::None;

  return static_cast<Level>(immune);
}

void Stagger::SetImmuneLevel(RE::Actor* actor, Level level)
{
  if (!actor)
    return;

  if (level > Level::Largest)
    level = Level::Largest;

  actor->SetGraphVariableInt(STAGGER_IMMUNE, static_cast<std::int32_t>(level));
}

void Stagger::ProcessStagger(RE::Actor* aggressor, RE::Actor* victim)
{
  if (!aggressor || !victim)
    return;

  // 不清除Map中的数据，直到EndTarget事件，以确保在攻击过程中持续生效

  Level currentLevel = GetStaggerLevel(victim);
  // 如果当前已经位于Rim Combat的额外硬直等级中直接返回
  if (currentLevel > Level::Largest)
    return;

  Level targetLevel = Level::None;
  {
    std::scoped_lock lock(mtx_targetCache);
    std::int8_t modifiedResult = 0;
    if (auto it = modifyTargetLevelMap.find(aggressor); it != modifyTargetLevelMap.end())
      modifiedResult = static_cast<std::int8_t>(currentLevel) + it->second;

    // 确保修改后的等级在合法范围内
    if (modifiedResult < 0)
      targetLevel = Level::None;
    else if (modifiedResult > static_cast<std::int8_t>(Level::Largest))
      targetLevel = Level::Largest;

    // 仅有设置等级才可以设置Rim Combat的硬直等级
    // 保证设置Rim Combat的硬直等级一定会生效
    Level setLevel = Level::None;
    if (auto it = setTargetLevelMap.find(aggressor); it != setTargetLevelMap.end())
      setLevel = it->second;

    targetLevel = setLevel > targetLevel ? setLevel : targetLevel;
  }

  // 如果目标等级为None，则不处理硬直
  if (targetLevel == Level::None)
    return;

  SetStaggerLevel(victim, targetLevel);
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
  setTargetLevelMap[actor] = level;
}

void Stagger::TargetModify(RE::Actor* actor, const std::string& payload)
{
  if (!actor)
    return;

  // 如果是战技的属性，那仅在Eligible状态生效，Subordinate状态不生效
  if (WeaponArt::Manager::GetPerform(actor) == WeaponArt::Manager::Perform::Subordinate)
    return;

  auto modifiedLevel = Utils::toInt(payload);
  if (modifiedLevel == 0)
    return;
  std::lock_guard lock(mtx_targetCache);
  modifyTargetLevelMap[actor] = modifiedLevel;
}

void Stagger::TargetEnd(RE::Actor* actor)
{
  if (!actor)
    return;

  std::lock_guard lock(mtx_targetCache);
  setTargetLevelMap.erase(actor);
  modifyTargetLevelMap.erase(actor);
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

  if (immuneLevel == Level::None || duration <= 0)
    return;

  SetImmuneLevel(actor, immuneLevel);
  std::lock_guard lock(mtx_immuneCache);
  immuneActors[actor] = Utils::GetTime<std::chrono::milliseconds>() + duration;
}

void Stagger::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload.starts_with("set|")) {
    auto level = static_cast<Level>(Utils::toInt(payload.substr(4)));
    if (level != Level::None)
      SetStaggerLevel(actor, level);
  } else if (payload == "end")
    actor->SetGraphVariableInt(STAGGER_LEVEL, 0);
  else if (payload.starts_with("targetset|"))
    TargetSet(actor, payload.substr(10));
  else if (payload.starts_with("targetmodify|"))
    TargetModify(actor, payload.substr(13));
  else if (payload == "targetend")
    TargetEnd(actor);
  else if (payload.starts_with("immune|"))
    Immune(actor, payload.substr(7));
}