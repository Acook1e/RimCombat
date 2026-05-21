#pragma once

enum class Race : std::uint8_t
{
  // 野猪，蓝客灵骑乘野猪 具有相同行为图
  // 查鲁斯，查鲁斯收割者具有相同行为图
  // 狗，狐狸，狼具有相同行为图
  // 蜘蛛，巨型蜘蛛，大型蜘蛛具有相同行为图
  // 熊人，狼人形态具有相同行为图

  None             = 0,
  Human            = 0x01,
  AshHopper        = 0x02,
  Bear             = 0x03,
  Boar             = 0x04,
  BoarMounted      = 0x05,
  Chaurus          = 0x06,
  ChaurusHunter    = 0x07,
  ChaurusReaper    = 0x08,
  Chicken          = 0x09,
  Cow              = 0x0A,
  Deer             = 0x0B,
  Dog              = 0x0C,
  Dragon           = 0x0D,
  DragonPriest     = 0x0E,
  Draugr           = 0x0F,
  DwarvenBallista  = 0x10,
  DwarvenCenturion = 0x11,
  DwarvenSphere    = 0x12,
  DwarvenSpider    = 0x13,
  Falmer           = 0x14,
  FlameAtronach    = 0x15,
  Fox              = 0x16,
  FrostAtronach    = 0x17,
  Gargoyle         = 0x18,
  Giant            = 0x19,
  GiantSpider      = 0x1A,
  Goat             = 0x1B,
  Hagraven         = 0x1C,
  Hare             = 0x1D,
  Horker           = 0x1E,
  Horse            = 0x1F,
  IceWraith        = 0x20,
  LargeSpider      = 0x21,
  Lurker           = 0x22,
  Mammoth          = 0x23,
  Mudcrab          = 0x24,
  Netch            = 0x25,
  Riekling         = 0x26,
  Sabrecat         = 0x27,
  Seeker           = 0x28,
  Skeever          = 0x29,
  Slaughterfish    = 0x2A,
  Spider           = 0x2B,
  Spriggan         = 0x2C,
  StormAtronach    = 0x2D,
  Troll            = 0x2E,
  VampireLord      = 0x2F,
  Werebear         = 0x30,
  Werewolf         = 0x31,
  Wisp             = 0x32,
  Wispmother       = 0x33,
  Wolf             = 0x34,
};

Race GetRace(RE::Actor* actor);