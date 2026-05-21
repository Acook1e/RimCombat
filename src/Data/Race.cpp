#include "Data/Race.h"

Race GetRace(RE::Actor* actor)
{
  static std::unordered_map<std::string_view, Race> raceMap = {
      {"0_Master.hkx", Race::Human},
      {"WolfBehavior.hkx", Race::Wolf},
      {"DogBehavior.hkx", Race::Dog},
      {"ChickenBehavior.hkx", Race::Chicken},
      {"HareBehavior.hkx", Race::Hare},
      {"AtronachFlameBehavior.hkx", Race::FlameAtronach},
      {"AtronachFrostBehavior.hkx", Race::FrostAtronach},
      {"AtronachStormBehavior.hkx", Race::StormAtronach},
      {"BearBehavior.hkx", Race::Bear},
      {"ChaurusBehavior.hkx", Race::Chaurus},
      {"H-CowBehavior.hkx", Race::Cow},
      {"DeerBehavior.hkx", Race::Deer},
      {"CHaurusFlyerBehavior.hkx", Race::ChaurusHunter},
      {"VampireBruteBehavior.hkx", Race::Gargoyle},
      {"BenthicLurkerBehavior.hkx", Race::Lurker},
      {"BoarBehavior.hkx", Race::Boar},
      {"BCBehavior.hkx", Race::DwarvenBallista},
      {"HMDaedra.hkx", Race::Seeker},
      {"NetchBehavior.hkx", Race::Netch},
      {"RieklingBehavior.hkx", Race::Riekling},
      {"ScribBehavior.hkx", Race::AshHopper},
      {"DragonBehavior.hkx", Race::Dragon},
      {"Dragon_Priest.hkx", Race::DragonPriest},
      {"DraugrBehavior.hkx", Race::Draugr},
      {"SCBehavior.hkx", Race::DwarvenSphere},
      {"DwarvenSpiderBehavior.hkx", Race::DwarvenSpider},
      {"SteamBehavior.hkx", Race::DwarvenCenturion},
      {"FalmerBehavior.hkx", Race::Falmer},
      {"FrostbiteSpiderBehavior.hkx", Race::Spider},
      {"GiantBehavior.hkx", Race::Giant},
      {"GoatBehavior.hkx", Race::Goat},
      {"HavgravenBehavior.hkx", Race::Hagraven},
      {"HorkerBehavior.hkx", Race::Horker},
      {"HorseBehavior.hkx", Race::Horse},
      {"IceWraithBehavior.hkx", Race::IceWraith},
      {"MammothBehavior.hkx", Race::Mammoth},
      {"MudcrabBehavior.hkx", Race::Mudcrab},
      {"SabreCatBehavior.hkx", Race::Sabrecat},
      {"SkeeverBehavior.hkx", Race::Skeever},
      {"SlaughterfishBehavior.hkx", Race::Slaughterfish},
      {"SprigganBehavior.hkx", Race::Spriggan},
      {"TrollBehavior.hkx", Race::Troll},
      {"VampireLord.hkx", Race::VampireLord},
      {"WerewolfBehavior.hkx", Race::Werewolf},
      {"WispBehavior.hkx", Race::Wispmother},
      {"WitchlightBehavior.hkx", Race::Wisp},
  };

  auto behaviorPath =
      actor->GetRace()->rootBehaviorGraphNames[actor->GetActorBase()->IsFemale() ? 1 : 0].data();
  auto behaviorName = std::filesystem::path(behaviorPath).filename().string();

  auto res = Race::None;
  if (auto it = raceMap.find(behaviorName); it != raceMap.end()) {
    res = it->second;
  } else {
    logger::warn("Execution::GetRace: Unknown behavior graph: {}", behaviorName);
    return Race::None;
  }

  if (res == Race::Human)
    return res;

  auto editorId = std::string{actor->GetRace()->GetFormEditorID()};
  switch (res) {
  case Race::Boar:
    static const auto DLC2RieklingMountedKeyword =
        RE::TESDataHandler::GetSingleton()->LookupForm<RE::BGSKeyword>(0x03A159, "Dragonborn.esm");
    if (actor->GetRace()->HasKeyword(DLC2RieklingMountedKeyword))
      res = Race::BoarMounted;
    break;
  case Race::Chaurus:
    if (editorId.find("reaper") != std::string::npos)
      res = Race::ChaurusReaper;
    else
      res = Race::Chaurus;
    break;
  case Race::Spider:
    if (editorId.find("giant") != std::string::npos)
      res = Race::GiantSpider;
    else if (editorId.find("large") != std::string::npos)
      res = Race::LargeSpider;
    break;
  case Race::Wolf:
    if (editorId.find("fox") != std::string::npos)
      res = Race::Fox;
    break;
  case Race::Werewolf:
    if (editorId.find("werebear") != std::string::npos)
      res = Race::Werebear;
    break;
  default:
    break;
  }
  return res;
}