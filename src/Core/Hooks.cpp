#include "Core/Hooks.h"

#include "Combat/Block.h"
#include "Combat/Exhausted.h"
#include "Combat/Posture.h"
#include "Utils.h"

namespace Hooks
{
float Hook_OnGetAttackStaminaCost::GetAttackStaminaCost(
    RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData)
{
  // REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  // RE::Actor* actor = &REL::RelocateMember<RE::Actor>(avOwner,
  // offset.offset());

  if (Settings::bUseAttackStaminaSystem)
    return 0.0f;

  return _GetAttackStaminaCost(avOwner, atkData);
}
void Hook_OnMeleeHit::ProcessHit(RE::Actor* victim, RE::HitData& hitData)
{
  auto aggressor = hitData.aggressor.get().get();

  if (!victim || !aggressor || victim->IsDead())
    return _ProcessHit(victim, hitData);

  auto timedBlock = false;
  if (Settings::bTimedBlockEnabled &&
      hitData.flags.any(RE::HitData::Flag::kBlocked))
    timedBlock = Block::GetSingleton().IsTimedBlock(victim);

  if (Settings::bUsePostureSystem)
    Posture::GetSingleton().ProcessMeleeHit(aggressor, victim, hitData,
                                            timedBlock);

  _ProcessHit(victim, hitData);
}
void Hook_OnModActorValue::ModActorValue_NPC(RE::ActorValueOwner* avOwner,
                                             RE::ACTOR_VALUE_MODIFIER modifier,
                                             RE::ActorValue akValue,
                                             float value)
{
  REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  RE::Actor* actor = &REL::RelocateMember<RE::Actor>(avOwner, offset.offset());
  value            = ModActorValue(actor, modifier, akValue, value);
  _ModActorValue_NPC(avOwner, modifier, akValue, value);
}
void Hook_OnModActorValue::ModActorValue_PC(RE::ActorValueOwner* avOwner,
                                            RE::ACTOR_VALUE_MODIFIER modifier,
                                            RE::ActorValue akValue, float value)
{
  REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  RE::Actor* actor = &REL::RelocateMember<RE::Actor>(avOwner, offset.offset());
  value            = ModActorValue(actor, modifier, akValue, value);
  _ModActorValue_PC(avOwner, modifier, akValue, value);
}
float Hook_OnModActorValue::ModActorValue(RE::Actor* actor,
                                          RE::ACTOR_VALUE_MODIFIER modifier,
                                          RE::ActorValue akValue, float value)
{
  switch (modifier) {
  case RE::ACTOR_VALUE_MODIFIER::kPermanent:
  case RE::ACTOR_VALUE_MODIFIER::kTemporary:
    return ModMaxActorValue(actor, akValue, value);
  case RE::ACTOR_VALUE_MODIFIER::kDamage:
    return ModCurrentActorValue(actor, akValue, value);
  default:
    return value;
  }
}
float Hook_OnModActorValue::ModMaxActorValue(RE::Actor* actor,
                                             RE::ActorValue akValue,
                                             float value)
{
  switch (akValue) {
  case RE::ActorValue::kHealth:
    if (Settings::bUsePostureSystem)
      Posture::GetSingleton().ReCalculateMaxPosture(actor);
    break;
  }
  return value;
}
float Hook_OnModActorValue::ModCurrentActorValue(RE::Actor* actor,
                                                 RE::ActorValue akValue,
                                                 float value)
{
  // Process ActorValue Decrease
  if (value < 0) {
    // Process Exhausted State Entry
    if (akValue == RE::ActorValue::kStamina) {
      if (Settings::bEnableExhausted &&
          actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) +
                  value <=
              0) {
        Exhausted::EnterExhausted(actor);
      }
    }
    // Process player god mode
    if (actor->IsPlayerRef() &&
        RE::PlayerCharacter::GetSingleton()->IsGodMode()) {
      switch (akValue) {
      case RE::ActorValue::kHealth:
      case RE::ActorValue::kStamina:
      case RE::ActorValue::kMagicka:
        return 0.0f;
      default:
        break;
      }
    }
    return value;
  }
  // Process ActorValue Increase
  switch (akValue) {
  case RE::ActorValue::kStamina: {
    // During attack, stamina does not regenerate
    RE::ATTACK_STATE_ENUM atkState = actor->AsActorState()->GetAttackState();
    if (atkState > RE::ATTACK_STATE_ENUM::kNone &&
        atkState <= RE::ATTACK_STATE_ENUM::kBowFollowThrough) {
      return 0.0f;
    }
    // Combat Multiplier
    if (actor->IsInCombat()) {
      value *= Settings::fStaminaRegenMultCombat;
    }
    // Block Multiplier
    if (actor->IsBlocking()) {
      value *= Settings::fStaminaRegenMultBlock;
    }
    // Process Exhausted State Exit
    if (Settings::bEnableExhausted && Exhausted::IsActorExhausted(actor)) {
      if (actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) /
              Utils::GetCurrentMaxActorValue(actor, RE::ActorValue::kStamina) >=
          Settings::fExhaustedExitPercent)
        Exhausted::ExitExhausted(actor);
    }
  } break;
  default:
    break;
  }
  return value;
}
}  // namespace Hooks