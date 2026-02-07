#include "Hooks.h"

#include "Posture.h"
#include "Stamina.h"
#include "Utils.h"

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
float Hook_OnModActorValue::ModActorValue(RE::Actor* a_actor, RE::ActorValue a_akValue, float a_value)
{
  if (a_value < 0) {
    if (a_akValue == RE::ActorValue::kStamina) {
      if (Settings::bEnableExhausted && a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) + a_value <= 0) {
        Posture::GetSingleton().EnterExhausted(a_actor);
      }
    }
    return a_value;
  }
  switch (a_akValue) {
  case RE::ActorValue::kStamina: {
    RE::ATTACK_STATE_ENUM atkState = a_actor->AsActorState()->GetAttackState();
    if (atkState > RE::ATTACK_STATE_ENUM::kNone && atkState <= RE::ATTACK_STATE_ENUM::kBowFollowThrough) {
      return 0;
    }
    if (a_actor->IsBlocking()) {
      a_value *= Settings::fStaminaRegenMultBlock;
    }
    if (Settings::bEnableExhausted && Posture::GetSingleton().IsActorExhausted(a_actor)) {
      if (a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) / Utils::GetCurrentMaxActorValue(a_actor, RE::ActorValue::kStamina) >=
          Settings::fExhaustedRestorePercent)
        Posture::GetSingleton().QuitExhausted(a_actor);
    }
  } break;
  case RE::ActorValue::kHealth:
    break;
  }
  return a_value;
}
void Hook_OnModActorValue::ModActorValue_NPC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier, RE::ActorValue a_akValue,
                                             float a_value)
{
  if (a_modifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
    REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
    RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
    a_value            = ModActorValue(a_actor, a_akValue, a_value);
  }
  _ModActorValue_NPC(a_this, a_modifier, a_akValue, a_value);
}
void Hook_OnModActorValue::ModActorValue_PC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier, RE::ActorValue a_akValue, float a_value)
{
  if (a_modifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
    REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
    RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
    a_value            = ModActorValue(a_actor, a_akValue, a_value);
  }
  _ModActorValue_PC(a_this, a_modifier, a_akValue, a_value);
}
}  // namespace Hooks