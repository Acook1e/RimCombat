#include "Combat/Stagger.h"

#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Utils.h"

#include "magic_enum/magic_enum.hpp"

using Level = Stagger::Level;

float Stagger::GetStaggerMagnitudeFromMap(Level level)
{
  if (level >= Level::Largest)
    return staggerMagnitudeMap[Level::Largest];

  if (auto it = staggerMagnitudeMap.find(level); it != staggerMagnitudeMap.end())
    return it->second;

  return 0.0f;
}

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
    std::lock_guard<std::mutex> lock(mtx);
    setTargetLevelMap.clear();
    modifyTargetLevelMap.clear();
  });
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

void Stagger::SetStaggerMagnitude(RE::Actor* actor, float magnitude)
{
  if (!actor)
    return;

  actor->SetGraphVariableFloat("staggerMagnitude", magnitude);
}

Level Stagger::GetStaggerLevel(RE::Actor* actor)
{
  if (!actor)
    return Level::None;

  float staggerMagnitude = GetStaggerMagnitude(actor);

  if (staggerMagnitude >= staggerMagnitudeMap[Level::Largest]) {

    auto staggerLevel = 0;
    if (!actor->GetGraphVariableInt(STAGGER_LEVEL, staggerLevel))
      return Level::Largest;
    if (staggerLevel > static_cast<std::int8_t>(Level::Largest))
      return static_cast<Level>(staggerLevel);
    return Level::Largest;
  } else if (staggerMagnitude >= staggerMagnitudeMap[Level::Large])
    return Level::Large;
  else if (staggerMagnitude >= staggerMagnitudeMap[Level::Medium])
    return Level::Medium;
  else if (staggerMagnitude >= staggerMagnitudeMap[Level::Small])
    return Level::Small;

  // 基于Modern Stagger Lock的配置，小于Small是有可能的，因此返回None而不是Small
  return Level::None;
}

void Stagger::SetStaggerLevel(RE::Actor* actor, Level level)
{
  if (!actor)
    return;

  if (level > Level::Largest)
    actor->SetGraphVariableInt(STAGGER_LEVEL, static_cast<std::int8_t>(level));
  else
    actor->SetGraphVariableInt(STAGGER_LEVEL, 0);

  float magnitude = GetStaggerMagnitudeFromMap(level);
  SetStaggerMagnitude(actor, magnitude);
}

void Stagger::ModifyStaggerLevel(RE::Actor* actor, std::int8_t modifiedLevel)
{
  if (!actor)
    return;

  auto currentLevel = GetStaggerLevel(actor);
  auto newLevel     = static_cast<std::int8_t>(currentLevel) + modifiedLevel;
  if (newLevel < static_cast<std::int8_t>(Level::None))
    newLevel = static_cast<std::int8_t>(Level::None);
  else if (newLevel > static_cast<std::int8_t>(Level::Largest))
    newLevel = static_cast<std::int8_t>(Level::Largest);

  SetStaggerLevel(actor, static_cast<Level>(newLevel));
}

bool Stagger::IsStaggerImmune(RE::Actor* actor)
{
  if (!actor)
    return false;

  auto immune  = GetStaggerImmuneLevel(actor);
  auto stagger = GetStaggerLevel(actor);

  if (immune >= stagger)
    return true;

  return false;
}

Level Stagger::GetStaggerImmuneLevel(RE::Actor* actor)
{
  if (!actor)
    return Level::None;

  std::int32_t immune = 0;
  if (!actor->GetGraphVariableInt(STAGGER_IMMUNE, immune))
    return Level::None;

  auto level = static_cast<Level>(immune);

  if (level > Level::Largest)
    return Level::Largest;

  return level;
}

void Stagger::SetStaggerImmuneLevel(RE::Actor* actor, Level level)
{
  if (!actor)
    return;

  if (level > Level::Largest)
    level = Level::Largest;

  actor->SetGraphVariableInt(STAGGER_IMMUNE, static_cast<std::int32_t>(level));
}

bool Stagger::ProcessStagger(RE::Actor* aggressor, RE::Actor* victim)
{
  if (!aggressor || !victim)
    return false;

  auto res = false;

  // 不清除Map中的数据，直到EndTarget事件，以确保在攻击过程中持续生效
  std::lock_guard lock(mtx);

  if (auto it = modifyTargetLevelMap.find(aggressor); it != modifyTargetLevelMap.end()) {
    res = true;
    ModifyStaggerLevel(victim, it->second);
  }

  // SetTarget的优先级高于ModifyTarget，如果同时存在Set和Modify，则只会Set目标的硬直等级，Modify会被忽略
  if (auto it = setTargetLevelMap.find(aggressor); it != setTargetLevelMap.end()) {
    res = true;
    SetStaggerLevel(victim, it->second);
  }

  return res;
}

void Stagger::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload == "end") {
    actor->SetGraphVariableInt(STAGGER_LEVEL, 0);
  } else if (payload.starts_with("immune|")) {
    auto immuneLevelStr = payload.substr(7);
    auto immuneLevel    = static_cast<Level>(Utils::toInt(immuneLevelStr));
    SetStaggerImmuneLevel(actor, immuneLevel);
  } else if (payload == "immuneend") {
    SetStaggerImmuneLevel(actor, Level::None);
  } else if (payload.starts_with("targetset|")) {
    auto levelStr = payload.substr(10);
    auto level    = static_cast<Level>(Utils::toInt(levelStr));
    std::lock_guard lock(mtx);
    if (level != Level::None)
      setTargetLevelMap[actor] = level;
  } else if (payload.starts_with("targetmodify|")) {
    auto levelStr = payload.substr(13);
    auto level    = static_cast<std::int8_t>(Utils::toInt(levelStr));
    std::lock_guard lock(mtx);
    modifyTargetLevelMap[actor] = level;
  } else if (payload == "targetend") {
    std::lock_guard lock(mtx);
    setTargetLevelMap.erase(actor);
    modifyTargetLevelMap.erase(actor);
  }
}