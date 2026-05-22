#include "Data/Race.h"

#include "magic_enum/magic_enum.hpp"

namespace Race
{

Type GetRace(RE::Actor* actor)
{
  static std::unordered_map<std::string_view, Type> raceMap = {
      {"0_Master.hkx", Type::Human},
      {"WolfBehavior.hkx", Type::Wolf},
      {"DogBehavior.hkx", Type::Dog},
      {"ChickenBehavior.hkx", Type::Chicken},
      {"HareBehavior.hkx", Type::Hare},
      {"AtronachFlameBehavior.hkx", Type::FlameAtronach},
      {"AtronachFrostBehavior.hkx", Type::FrostAtronach},
      {"AtronachStormBehavior.hkx", Type::StormAtronach},
      {"BearBehavior.hkx", Type::Bear},
      {"ChaurusBehavior.hkx", Type::Chaurus},
      {"H-CowBehavior.hkx", Type::Cow},
      {"DeerBehavior.hkx", Type::Deer},
      {"CHaurusFlyerBehavior.hkx", Type::ChaurusHunter},
      {"VampireBruteBehavior.hkx", Type::Gargoyle},
      {"BenthicLurkerBehavior.hkx", Type::Lurker},
      {"BoarBehavior.hkx", Type::Boar},
      {"BCBehavior.hkx", Type::DwarvenBallista},
      {"HMDaedra.hkx", Type::Seeker},
      {"NetchBehavior.hkx", Type::Netch},
      {"RieklingBehavior.hkx", Type::Riekling},
      {"ScribBehavior.hkx", Type::AshHopper},
      {"DragonBehavior.hkx", Type::Dragon},
      {"Dragon_Priest.hkx", Type::DragonPriest},
      {"DraugrBehavior.hkx", Type::Draugr},
      {"SCBehavior.hkx", Type::DwarvenSphere},
      {"DwarvenSpiderBehavior.hkx", Type::DwarvenSpider},
      {"SteamBehavior.hkx", Type::DwarvenCenturion},
      {"FalmerBehavior.hkx", Type::Falmer},
      {"FrostbiteSpiderBehavior.hkx", Type::Spider},
      {"GiantBehavior.hkx", Type::Giant},
      {"GoatBehavior.hkx", Type::Goat},
      {"HavgravenBehavior.hkx", Type::Hagraven},
      {"HorkerBehavior.hkx", Type::Horker},
      {"HorseBehavior.hkx", Type::Horse},
      {"IceWraithBehavior.hkx", Type::IceWraith},
      {"MammothBehavior.hkx", Type::Mammoth},
      {"MudcrabBehavior.hkx", Type::Mudcrab},
      {"SabreCatBehavior.hkx", Type::Sabrecat},
      {"SkeeverBehavior.hkx", Type::Skeever},
      {"SlaughterfishBehavior.hkx", Type::Slaughterfish},
      {"SprigganBehavior.hkx", Type::Spriggan},
      {"TrollBehavior.hkx", Type::Troll},
      {"VampireLord.hkx", Type::VampireLord},
      {"WerewolfBehavior.hkx", Type::Werewolf},
      {"WispBehavior.hkx", Type::Wispmother},
      {"WitchlightBehavior.hkx", Type::Wisp},
  };

  auto behaviorPath =
      actor->GetRace()->rootBehaviorGraphNames[actor->GetActorBase()->IsFemale() ? 1 : 0].data();
  auto behaviorName = std::filesystem::path(behaviorPath).filename().string();

  auto res = Type::None;
  if (auto it = raceMap.find(behaviorName); it != raceMap.end()) {
    res = it->second;
  } else {
    logger::warn("Execution::GetRace: Unknown behavior graph: {}", behaviorName);
    return Type::None;
  }

  if (res == Type::Human)
    return res;

  auto editorId = std::string{actor->GetRace()->GetFormEditorID()};
  switch (res) {
  case Type::Boar:
    static const auto DLC2RieklingMountedKeyword =
        RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x03A159, "Dragonborn.esm");
    if (actor->GetRace()->HasKeyword(DLC2RieklingMountedKeyword))
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

}  // namespace Race