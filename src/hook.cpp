#include "hook.h"

#include "poiseHandler.h"

namespace Hooks
{
float Hook_OnGetAttackStaminaCost::getAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData)
{
  auto offset        = REL::VariantOffset(-0xB0, -0xB8, 0x0);
  RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(avOwner, offset.offset());

  if (Settings::bPowerAttackComsumeStaminaTweak)
    return 0;

  if (!Settings::bConsumeStaminaOutCombat && !a_actor->IsInCombat()) {
    return 0;
  }
  return _getAttackStaminaCost(avOwner, atkData);
}

/*function generating conditions for stamina regen. If returned value is true,
no regen. used to block stamina regen in certain situations.*/
bool Hook_OnCheckStaminaRegenCondition::HasFlags1(RE::ActorState* a_this, uint16_t a_flags)
{
  // if bResult is true, prevents regen.
  bool bResult = _HasFlags1(a_this, a_flags);  // is sprinting?

  if (!bResult) {
    auto attackState = a_this->GetAttackState();

    // if melee hit regen is needed, no need to disable regen.
    bResult = (attackState > RE::ATTACK_STATE_ENUM::kNone && attackState <= RE::ATTACK_STATE_ENUM::kBowFollowThrough);
    // don't regen stamina if attacking
  }
  return bResult;
}

float Hook_OnModActorValue::ModActorValue(RE::Actor* a_actor, RE::ActorValue a_akValue, float a_value)
{
  if (a_value < 0)
    return a_value;
  switch (a_akValue) {
  case RE::ActorValue::kStamina:
    if (a_actor->IsBlocking()) {
      a_value *= Settings::fBlockingStaminaRegenMult;
    } else {
      RE::ATTACK_STATE_ENUM atkState = a_actor->AsActorState()->GetAttackState();
      if (atkState > RE::ATTACK_STATE_ENUM::kNone && atkState <= RE::ATTACK_STATE_ENUM::kBowFollowThrough) {
        return 0;
      }
    }
    break;
  case RE::ActorValue::kHealth:
    if (Settings::poiseType == PoiseType::kPoiseHandler) {
      if (Settings::bRestorePoiseInCombat && !a_actor->IsInCombat())
        break;
      if (Settings::executionMark && a_actor->HasSpell(Settings::executionMark))
        break;
      Handler::PoiseHandler::RimCombatPoise::GetSingleton().RestorePoiseHealth(a_actor, 0.0005f);
    }
    break;
  }
  return a_value;
}

void Hook_OnModActorValue::ModActorValue_NPC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier,
                                             RE::ActorValue a_akValue, float a_value)
{
  if (a_modifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
    auto offset        = REL::VariantOffset(-0xB0, -0xB8, 0x0);
    RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
    a_value            = ModActorValue(a_actor, a_akValue, a_value);
  }
  _ModActorValue_NPC(a_this, a_modifier, a_akValue, a_value);
}

void Hook_OnModActorValue::ModActorValue_PC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier,
                                            RE::ActorValue a_akValue, float a_value)
{
  if (a_modifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
    auto offset        = REL::VariantOffset(-0xB0, -0xB8, 0x0);
    RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
    a_value            = ModActorValue(a_actor, a_akValue, a_value);
  }
  _ModActorValue_PC(a_this, a_modifier, a_akValue, a_value);
}

void Hook_OnMeleeHit::processHit(RE::Actor* victim, RE::HitData& hitData)
{
  auto aggressor = hitData.aggressor.get().get();

  if (!victim || !aggressor || victim->IsDead())
    return _ProcessHit(victim, hitData);

  Handler::PoiseHandler::RimCombatPoise::GetSingleton().ProcessHit(aggressor, victim, hitData);
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