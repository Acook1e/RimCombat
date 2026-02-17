#include "Hooks.h"

#include "Block.h"
#include "Posture.h"
#include "Utils.h"

namespace Hooks
{
float Hook_OnGetAttackStaminaCost::GetAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData)
{
  // REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  // RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(avOwner, offset.offset());

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
  if (Settings::bTimedBlockEnabled && hitData.flags.any(RE::HitData::Flag::kBlocked))
    timedBlock = Block::GetSingleton().IsTimedBlock(victim);

  if (Settings::bUsePostureSystem)
    Posture::GetSingleton().ProcessMeleeHit(aggressor, victim, hitData, timedBlock);

  _ProcessHit(victim, hitData);
}
void Hook_OnModActorValue::ModActorValue_NPC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier, RE::ActorValue a_akValue,
                                             float a_value)
{
  REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
  a_value            = ModActorValue(a_actor, a_modifier, a_akValue, a_value);
  _ModActorValue_NPC(a_this, a_modifier, a_akValue, a_value);
}
void Hook_OnModActorValue::ModActorValue_PC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier, RE::ActorValue a_akValue, float a_value)
{
  REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
  a_value            = ModActorValue(a_actor, a_modifier, a_akValue, a_value);
  _ModActorValue_PC(a_this, a_modifier, a_akValue, a_value);
}
float Hook_OnModActorValue::ModActorValue(RE::Actor* a_actor, RE::ACTOR_VALUE_MODIFIER a_modifier, RE::ActorValue a_akValue, float a_value)
{
  switch (a_modifier) {
  case RE::ACTOR_VALUE_MODIFIER::kPermanent:
  case RE::ACTOR_VALUE_MODIFIER::kTemporary:
    return ModMaxActorValue(a_actor, a_akValue, a_value);
  case RE::ACTOR_VALUE_MODIFIER::kDamage:
    return ModCurrentActorValue(a_actor, a_akValue, a_value);
  default:
    return a_value;
  }
}
float Hook_OnModActorValue::ModMaxActorValue(RE::Actor* a_actor, RE::ActorValue a_akValue, float a_value)
{
  switch (a_akValue) {
  case RE::ActorValue::kHealth:
    if (Settings::bUsePostureSystem)
      Posture::GetSingleton().ReCalculateMaxPosture(a_actor);
    break;
  }
  return a_value;
}
float Hook_OnModActorValue::ModCurrentActorValue(RE::Actor* a_actor, RE::ActorValue a_akValue, float a_value)
{
  // Process ActorValue Decrease
  if (a_value < 0) {
    // Process Exhausted State Entry
    if (a_akValue == RE::ActorValue::kStamina) {
      if (Settings::bEnableExhausted && a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) + a_value <= 0) {
        Posture::GetSingleton().EnterExhausted(a_actor);
      }
    }
    // Process player god mode
    if (a_actor->IsPlayerRef() && RE::PlayerCharacter::GetSingleton()->IsGodMode()) {
      switch (a_akValue) {
      case RE::ActorValue::kHealth:
      case RE::ActorValue::kStamina:
      case RE::ActorValue::kMagicka:
        return 0.0f;
      default:
        break;
      }
    }
    return a_value;
  }
  // Process ActorValue Increase
  switch (a_akValue) {
  case RE::ActorValue::kStamina: {
    // During attack, stamina does not regenerate
    RE::ATTACK_STATE_ENUM atkState = a_actor->AsActorState()->GetAttackState();
    if (atkState > RE::ATTACK_STATE_ENUM::kNone && atkState <= RE::ATTACK_STATE_ENUM::kBowFollowThrough) {
      return 0.0f;
    }
    // Combat Multiplier
    if (a_actor->IsInCombat()) {
      a_value *= Settings::fStaminaRegenMultCombat;
    }
    // Block Multiplier
    if (a_actor->IsBlocking()) {
      a_value *= Settings::fStaminaRegenMultBlock;
    }
    // Process Exhausted State Exit
    if (Settings::bEnableExhausted && Posture::GetSingleton().IsActorExhausted(a_actor)) {
      if (a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) / Utils::GetCurrentMaxActorValue(a_actor, RE::ActorValue::kStamina) >=
          Settings::fExhaustedRestorePercent)
        Posture::GetSingleton().ExitExhausted(a_actor);
    }
  } break;
  default:
    break;
  }
  return a_value;
}
}  // namespace Hooks