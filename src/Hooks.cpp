#include "Hooks.h"

#include "Posture.h"
#include "Stamina.h"

namespace Hooks
{
float Hook_OnGetAttackStaminaCost::GetAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData)
{
  REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(avOwner, offset.offset());

  if (Settings::bUseAttackStaminaSystem)
    return Stamina::AttackStaminaConsume(a_actor, true, false);

  return _GetAttackStaminaCost(avOwner, atkData);
}
void Hook_OnMeleeHit::ProcessHit(RE::Actor* victim, RE::HitData& hitData)
{
  auto aggressor = hitData.aggressor.get().get();

  if (!victim || !aggressor || victim->IsDead())
    return _ProcessHit(victim, hitData);

  if (Settings::bUsePostureSystem)
    Posture::GetSingleton().ProcessMeleeHit(aggressor, victim, hitData);

  _ProcessHit(victim, hitData);
}
}  // namespace Hooks