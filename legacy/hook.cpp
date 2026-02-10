#include "hook.h"

#include "attackHandler.h"
#include "poiseHandler.h"

namespace Hooks
{
void Hook_OnMeleeHit::processHit(RE::Actor* victim, RE::HitData& hitData)
{
  auto aggressor = hitData.aggressor.get().get();

  if (!victim || !aggressor || victim->IsDead())
    return _ProcessHit(victim, hitData);

  if (Settings::bEnablePoiseExecution)
    Handler::Poise::RimPoise::GetSingleton().ProcessMeleeHit(aggressor, victim, hitData);

  if (Settings::bUseExhaustionSystem && Handler::Attack::GetSingleton().IsInExhaustion(aggressor)) {
    hitData.totalDamage *= Settings::fExhaustionDamageMult;
  }

  _ProcessHit(victim, hitData);
}

/*Check if the attack action should be performed depending on the actor's debuff
 * state.*/
bool Hook_OnAttackAction::PerformAttackAction(RE::TESActionData* a_actionData)
{
  // auto offset      = REL::VariantOffset(-0xB8, -0xC0, 0x0);
  // RE::Actor* actor = &REL::RelocateMember<RE::Actor>(a_actionData->GetSourceActorState(), offset.offset());

  // std::string event = a_actionData->animEvent.data();
  // logger::info("IsPlayer {} Current Event {}", actor->IsPlayerRef(), event);

  return _PerformAttackAction(a_actionData);
}
}  // namespace Hooks