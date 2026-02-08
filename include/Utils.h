#pragma once

namespace Utils
{
float GetCurrentMaxActorValue(RE::Actor* actor, RE::ActorValue av);

constexpr std::uint32_t hash(const char* data, size_t const size) noexcept
{
  uint32_t hash = nexusID;

  for (const char* c = data; c < data + size; ++c) {
    hash = ((hash << 5) + hash) + (unsigned char)*c;
  }

  return hash;
}
constexpr std::uint32_t operator""_h(const char* str, size_t size) noexcept
{
  return hash(str, size);
}
}  // namespace Utils