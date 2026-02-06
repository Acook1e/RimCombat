#include "Utils.h"

namespace Utils
{
float GetCurrentMaxActorValue(RE::Actor* actor, RE::ActorValue av)
{
  return actor->AsActorValueOwner()->GetPermanentActorValue(av) + actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
}
}  // namespace Utils