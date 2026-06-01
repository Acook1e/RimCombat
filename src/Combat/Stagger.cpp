#include "Combat/Stagger.h"

#include "Combat/WeaponArt.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Utils.h"

// 对于uint8的枚举类型，magic_enum默认会将其转换为int进行处理，导致无法正确转换回枚举值
// 因此需要在包含magic_enum之前定义MAGIC_ENUM_RANGE_MIN和MAGIC_ENUM_RANGE_MAX来指定枚举的范围
#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 256
#include "magic_enum/magic_enum.hpp"
#include "nlohmann/json.hpp"

using Level = Stagger::Level;

Stagger::Stagger()
{

  // 加载法术命中的硬直等级
  const auto loadStaggerSettings = []() {
    const std::string staggerSettingsDir = Settings::SettingsDir + "Stagger/";

    auto* dataHandle = RE::TESDataHandler::GetSingleton();
    if (!dataHandle) {
      logger::error("Failed to get TESDataHandler singleton.");
      return;
    }

    for (const auto& entry : std::filesystem::directory_iterator(staggerSettingsDir)) {
      if (!entry.is_regular_file() || entry.path().extension() != ".json")
        continue;
      try {
        std::ifstream file(entry.path());
        nlohmann::json j;
        file >> j;

        if (!j.is_array()) {
          logger::warn("Invalid stagger settings format in file {}. Expected an array. Skipping.",
                       entry.path().string());
          continue;
        }

        for (const auto& item : j) {
          std::string mod       = item.value("mod", "");
          std::string formIDStr = item.value("formID", "");
          auto formID           = Utils::toInt(formIDStr, 16);
          auto staggerLevelStr  = item.value("level", "None");
          auto staggerLevel = magic_enum::enum_cast<Level>(staggerLevelStr).value_or(Level::None);
          if (staggerLevel == Level::None) {
            logger::warn("Invalid stagger level '{}' in file {}. Skipping.", staggerLevelStr,
                         entry.path().string());
            continue;
          }
          auto form = dataHandle->LookupFormID(formID, mod);
          if (!form) {
            logger::warn("Invalid form ID '{}|{}' for stagger settings in file {}. Skipping.",
                         formIDStr, mod, entry.path().string());
            continue;
          }
          projectileStagger[form] = staggerLevel;
        }

      } catch (const std::exception& e) {
        logger::warn("Failed to load stagger settings from file {}: {}", entry.path().string(),
                     e.what());
      }
    }
  };

  // 加载Modern Stagger Lock的硬直等级对应的数值阈值
  const auto loadMSLSettings = []() {
    constexpr std::string_view MSLSettingsFile = "Data/SKSE/Plugins/ModernStaggerLock.ini";

    std::error_code ec;
    if (!std::filesystem::exists(MSLSettingsFile, ec)) {
      logger::warn("Stagger: Modern Stagger Lock settings file not found: {}", MSLSettingsFile);
      return;
    }

    auto path = std::filesystem::path(MSLSettingsFile);

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
      logger::warn("Stagger: failed to open Modern Stagger Lock settings file: {}",
                   MSLSettingsFile);
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
  };

  loadStaggerSettings();
  loadMSLSettings();

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
      if (it->second <= now)
        it = immuneActors.erase(it);
      else
        ++it;
    }
  }

  // 更新恢复时间缓存
  {
    std::scoped_lock lock(mtx_recover);
    for (auto it = staggerRecovery.begin(); it != staggerRecovery.end();) {
      auto victim               = it->first;
      auto [recoverTime, level] = it->second;
      // 特殊硬直的恢复时间有Recoverable事件来控制
      if (level < Level::Largest)
        continue;
      if (recoverTime <= now) {
        if (victim)
          victim->SetGraphVariableBool(STAGGER_RECOVERABLE, true);
        it = staggerRecovery.erase(it);
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

  auto staggerLevel = magic_enum::enum_cast<Level>(level);
  if (!staggerLevel.has_value())
    return Level::None;

  return staggerLevel.value();
}

void Stagger::SetStaggerLevel(RE::Actor* actor, Level level)
{
  if (!actor)
    return;

  actor->SetGraphVariableInt(STAGGER_LEVEL, static_cast<std::int32_t>(level));
}

bool Stagger::IsImmune(RE::Actor* actor)
{
  if (!actor)
    return false;

  std::lock_guard<std::mutex> lock(mtx_immuneCache);
  return immuneActors.contains(actor);
}

void Stagger::ProcessWeaponStagger(RE::Actor* aggressor, RE::Actor* victim)
{
  if (!aggressor || !victim || !Settings::bUseStaggerSystem)
    return;

  // 处理受击者硬直
  Level currentLevel = GetStaggerLevel(victim);

  // 处理外源硬直
  // 不清除Map中的数据，直到EndTarget事件，以确保在攻击过程中持续生效
  Level targetLevel = Level::None;
  {
    std::lock_guard lock(mtx_targetCache);
    if (auto it = staggerLevelOnHit.find(aggressor); it != staggerLevelOnHit.end())
      targetLevel = it->second;
  }

  // 基础硬直等级会被高等级的外源硬直覆盖
  if (targetLevel > Level::None && targetLevel < Level::Largest && targetLevel > currentLevel)
    currentLevel = targetLevel;

  SetStaggerLevel(victim, currentLevel);
  SetStaggerMagnitude(victim, currentLevel);
  StaggerStart(victim);
}

void Stagger::ProcessProjectileStagger(RE::Actor* victim, RE::FormID formID)
{
  if (!victim || !Settings::bUseStaggerSystem)
    return;

  if (auto it = projectileStagger.find(formID); it != projectileStagger.end()) {
    auto staggerLevel = it->second;
    if (staggerLevel == Level::None)
      return;
    SetStaggerLevel(victim, staggerLevel);
    SetStaggerMagnitude(victim, staggerLevel);
    StaggerStart(victim);
  }
}

void Stagger::StaggerStart(RE::Actor* victim)
{
  if (!victim || !Settings::bUseStaggerSystem)
    return;

  auto level = GetStaggerLevel(victim);
  if (level == Level::None)
    return;

  // 免疫GuardBreak以下的硬直等级
  if (level < Level::GuardBreak && IsImmune(victim))
    return;

  std::uint64_t recoverTime = 0;
  switch (level) {
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
    std::scoped_lock<std::mutex> lock(mtx_recover);
    if (!staggerRecovery.contains(victim))
      staggerRecovery[victim] = {now + recoverTime, level};
    else {
      auto [lastTime, lastLevel] = staggerRecovery[victim];

      // 如果在大硬直期间受到小硬直，则忽略这次硬直
      if (lastLevel > level)
        return;

      auto delta = lastTime - now;
      // TODO：找一个比相加乘以0.2更合理的算法
      recoverTime = (delta + recoverTime) * 0.2f;
      if (recoverTime < 100 || recoverTime > delta)
        recoverTime = 100;

      staggerRecovery[victim] = {now + recoverTime, level};

      // 受到同级别的硬直时，不再进入硬直动画
      if (lastLevel == level)
        return;
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

  // 允许免疫特殊等级，但处决硬直是无法被免疫的
  // 因此仍然允许设置处决硬直的免疫状态以实现特定敌人完全免疫硬直的效果
  if (immuneLevel == Level::None || duration <= 0)
    return;

  std::lock_guard lock(mtx_immuneCache);
  immuneActors[actor] = Utils::GetTime<std::chrono::milliseconds>() + duration;
}

void Stagger::Recoverable(RE::Actor* actor)
{
  if (!actor)
    return;

  std::lock_guard lock(mtx_recover);
  staggerRecovery.erase(actor);
  actor->SetGraphVariableBool(STAGGER_RECOVERABLE, true);
}

void Stagger::PayloadParse(RE::Actor* actor, const std::string& payload)
{
  if (payload.starts_with("targetset|"))
    TargetSet(actor, payload.substr(10));
  else if (payload == "targetend")
    TargetEnd(actor);
  else if (payload.starts_with("immune|"))
    Immune(actor, payload.substr(7));
  else if (payload == "recoverable")
    Recoverable(actor);
}