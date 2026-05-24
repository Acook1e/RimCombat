#include "Data/Race.h"

#include "Utils.h"

#include "magic_enum/magic_enum.hpp"

namespace Race
{

std::array<Type, 2> GetRaceImpl(RE::TESRace* race)
{
  static std::unordered_map<std::uint32_t, Type> raceMap = {
      {"0_Master.hkx"_h, Type::Human},
      {"WolfBehavior.hkx"_h, Type::Wolf},
      {"DogBehavior.hkx"_h, Type::Dog},
      {"ChickenBehavior.hkx"_h, Type::Chicken},
      {"HareBehavior.hkx"_h, Type::Hare},
      {"AtronachFlameBehavior.hkx"_h, Type::FlameAtronach},
      {"AtronachFrostBehavior.hkx"_h, Type::FrostAtronach},
      {"AtronachStormBehavior.hkx"_h, Type::StormAtronach},
      {"BearBehavior.hkx"_h, Type::Bear},
      {"ChaurusBehavior.hkx"_h, Type::Chaurus},
      {"H-CowBehavior.hkx"_h, Type::Cow},
      {"DeerBehavior.hkx"_h, Type::Deer},
      {"CHaurusFlyerBehavior.hkx"_h, Type::ChaurusHunter},
      {"VampireBruteBehavior.hkx"_h, Type::Gargoyle},
      {"BenthicLurkerBehavior.hkx"_h, Type::Lurker},
      {"BoarBehavior.hkx"_h, Type::Boar},
      {"BCBehavior.hkx"_h, Type::DwarvenBallista},
      {"HMDaedra.hkx"_h, Type::Seeker},
      {"NetchBehavior.hkx"_h, Type::Netch},
      {"RieklingBehavior.hkx"_h, Type::Riekling},
      {"ScribBehavior.hkx"_h, Type::AshHopper},
      {"DragonBehavior.hkx"_h, Type::Dragon},
      {"Dragon_Priest.hkx"_h, Type::DragonPriest},
      {"DraugrBehavior.hkx"_h, Type::Draugr},
      {"SCBehavior.hkx"_h, Type::DwarvenSphere},
      {"DwarvenSpiderBehavior.hkx"_h, Type::DwarvenSpider},
      {"SteamBehavior.hkx"_h, Type::DwarvenCenturion},
      {"FalmerBehavior.hkx"_h, Type::Falmer},
      {"FrostbiteSpiderBehavior.hkx"_h, Type::Spider},
      {"GiantBehavior.hkx"_h, Type::Giant},
      {"GoatBehavior.hkx"_h, Type::Goat},
      {"HavgravenBehavior.hkx"_h, Type::Hagraven},
      {"HorkerBehavior.hkx"_h, Type::Horker},
      {"HorseBehavior.hkx"_h, Type::Horse},
      {"IceWraithBehavior.hkx"_h, Type::IceWraith},
      {"MammothBehavior.hkx"_h, Type::Mammoth},
      {"MudcrabBehavior.hkx"_h, Type::Mudcrab},
      {"SabreCatBehavior.hkx"_h, Type::Sabrecat},
      {"SkeeverBehavior.hkx"_h, Type::Skeever},
      {"SlaughterfishBehavior.hkx"_h, Type::Slaughterfish},
      {"SprigganBehavior.hkx"_h, Type::Spriggan},
      {"TrollBehavior.hkx"_h, Type::Troll},
      {"VampireLord.hkx"_h, Type::VampireLord},
      {"WerewolfBehavior.hkx"_h, Type::Werewolf},
      {"WispBehavior.hkx"_h, Type::Wispmother},
      {"WitchlightBehavior.hkx"_h, Type::Wisp},
  };

  const auto getRaceFromBehavior = [&](const std::string& behaviorPath) -> Type {
    auto behaviorName = std::filesystem::path(behaviorPath).filename().string();
    auto hash         = Utils::hash(behaviorName);

    auto res = Type::None;
    if (auto it = raceMap.find(hash); it != raceMap.end()) {
      res = it->second;
    } else {
      logger::warn("GetRace: Unknown behavior graph: {}", behaviorName);
      return Type::None;
    }

    if (res == Type::Human)
      return res;

    auto editorId = std::string{race->GetFormEditorID()};
    switch (res) {
    case Type::Boar:
      static const auto DLC2RieklingMountedKeyword =
          RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x03A159,
                                                                         "Dragonborn.esm");
      if (race->HasKeyword(DLC2RieklingMountedKeyword))
        res = Type::BoarMounted;
      break;
    case Type::Chaurus:
      if (editorId.find("reaper") != std::string::npos)
        res = Type::ChaurusReaper;
      else
        res = Type::Chaurus;
      break;
    case Type::Spider:
      if (editorId.find("giant") != std::string::npos)
        res = Type::GiantSpider;
      else if (editorId.find("large") != std::string::npos)
        res = Type::LargeSpider;
      break;
    case Type::Wolf:
      if (editorId.find("fox") != std::string::npos)
        res = Type::Fox;
      break;
    case Type::Werewolf:
      if (editorId.find("werebear") != std::string::npos)
        res = Type::Werebear;
      break;
    default:
      break;
    }
    return res;
  };

  const std::string behaviorPathMale   = race->rootBehaviorGraphNames[0].data();
  const std::string behaviorPathFemale = race->rootBehaviorGraphNames[1].data();

  if (behaviorPathMale == behaviorPathFemale) {
    auto res = getRaceFromBehavior(behaviorPathMale);
    return {res, res};
  } else {
    auto resMale   = getRaceFromBehavior(behaviorPathMale);
    auto resFemale = getRaceFromBehavior(behaviorPathFemale);
    return {resMale, resFemale};
  }
}

Type GetRace(RE::Actor* actor)
{
  // 因为这个函数会被频繁调用，所以使用一个简单的缓存来避免重复计算
  static std::unordered_map<RE::TESRace*, std::array<Type, 2>> cache;

  if (!actor)
    return Type::None;

  auto race     = actor->GetRace();
  bool isFemale = actor->GetActorBase()->IsFemale();

  if (auto it = cache.find(race); it != cache.end())
    return it->second[isFemale ? 1 : 0];

  auto [resMale, resFemale] = GetRaceImpl(race);
  cache[race]               = {resMale, resFemale};
  return isFemale ? resFemale : resMale;
}

float GetBasePostureHealth(RE::Actor* actor)
{
  auto race = GetRace(actor);
  auto id   = static_cast<EnumType>(race);
  if (Settings::basePostureMap.contains(id))
    return Settings::basePostureMap[id];
  logger::warn("Race::GetBasePostureHealth: Unsupported race type: {}",
               magic_enum::enum_name(race));
  return 0.0f;
}

float GetBasePoiseHealth(RE::Actor* actor)
{
  auto race = GetRace(actor);
  auto id   = static_cast<EnumType>(race);
  if (Settings::basePoiseMap.contains(id))
    return Settings::basePoiseMap[id];
  logger::warn("Race::GetBasePoiseHealth: Unsupported race type: {}", magic_enum::enum_name(race));
  return 0.0f;
}

float GetBaseStaminaConsumption(Type type)
{
  auto id = static_cast<EnumType>(type);
  if (Settings::baseCreatureStaminaMap.contains(id))
    return Settings::baseCreatureStaminaMap[id];
  logger::warn("Race::GetBaseStaminaConsumption: Unsupported race type: {}",
               magic_enum::enum_name(type));
  return 0.0f;
}

float GetBasePoiseDamage(Type type)
{
  auto id = static_cast<EnumType>(type);
  if (Settings::baseCreaturePoiseDamage.contains(id))
    return Settings::baseCreaturePoiseDamage[id];
  logger::warn("Race::GetBasePoiseDamage: Unsupported race type: {}", magic_enum::enum_name(type));
  return 0.0f;
}

float GetBasePostureDamage(Type type)
{
  auto id = static_cast<EnumType>(type);
  if (Settings::baseCreaturePostureDamage.contains(id))
    return Settings::baseCreaturePostureDamage[id];
  logger::warn("Race::GetBasePostureDamage: Unsupported race type: {}",
               magic_enum::enum_name(type));
  return 0.0f;
}

}  // namespace Race