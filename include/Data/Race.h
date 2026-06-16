#pragma once

#include "Core/Settings.h"

namespace Race
{
using EnumType = Settings::RaceEnumType;

enum class Type : EnumType
{
  // 野猪，蓝客灵骑乘野猪 具有相同行为图
  // 查鲁斯，查鲁斯收割者具有相同行为图
  // 狗，狐狸，狼具有相同行为图
  // 蜘蛛，巨型蜘蛛，大型蜘蛛具有相同行为图
  // 熊人，狼人形态具有相同行为图

  None             = 0,
  Human            = 1,
  AshHopper        = 2,
  Bear             = 3,
  Boar             = 4,
  BoarMounted      = 5,
  Chaurus          = 6,
  ChaurusHunter    = 7,
  ChaurusReaper    = 8,
  Chicken          = 9,
  Cow              = 10,
  Deer             = 11,
  Dog              = 12,
  Dragon           = 13,
  DragonPriest     = 14,
  Draugr           = 15,
  DwarvenBallista  = 16,
  DwarvenCenturion = 17,
  DwarvenSphere    = 18,
  DwarvenSpider    = 19,
  Falmer           = 20,
  FlameAtronach    = 21,
  Fox              = 22,
  FrostAtronach    = 23,
  Gargoyle         = 24,
  Giant            = 25,
  GiantSpider      = 26,
  Goat             = 27,
  Hagraven         = 28,
  Hare             = 29,
  Horker           = 30,
  Horse            = 31,
  IceWraith        = 32,
  LargeSpider      = 33,
  Lurker           = 34,
  Mammoth          = 35,
  Mudcrab          = 36,
  Netch            = 37,
  Riekling         = 38,
  Sabrecat         = 39,
  Seeker           = 40,
  Skeever          = 41,
  Slaughterfish    = 42,
  Spider           = 43,
  Spriggan         = 44,
  StormAtronach    = 45,
  Troll            = 46,
  VampireLord      = 47,
  Werebear         = 48,
  Werewolf         = 49,
  Wisp             = 50,
  Wispmother       = 51,
  Wolf             = 52,
};

[[nodiscard]] Type GetRace(RE::Actor* actor);
[[nodiscard]] float GetBasePoiseHealth(RE::Actor* actor);
[[nodiscard]] float GetBasePostureHealth(RE::Actor* actor);

[[nodiscard]] float GetBaseStaminaConsumption(Type type);
[[nodiscard]] float GetBasePoiseDamage(Type type);
[[nodiscard]] float GetBasePostureDamage(Type type);
}  // namespace Race