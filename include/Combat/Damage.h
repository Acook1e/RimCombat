#pragma once

namespace Damage
{
enum class Type : std::uint32_t
{
  // 无类型伤害，不受任何抗性的影响
  // 可认为是真实伤害
  None    = 0,
  Normal  = 1 << 0,
  Fire    = 1 << 1,
  Shock   = 1 << 2,
  Frost   = 1 << 3,
  Magic   = 1 << 4,
  Poison  = 1 << 5,
  Disease = 1 << 6,
};

[[nodiscard]] inline float GetResisistance(RE::Actor* actor, Type type)
{
  return 0.0f;
}
}  // namespace Damage