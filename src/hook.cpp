#include "hook.h"

#include "attackHandler.h"
#include "poiseHandler.h"

namespace Hooks
{
float Hook_OnGetAttackStaminaCost::getAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData)
{
  REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
  RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(avOwner, offset.offset());

  if (!Settings::bConsumeStaminaOutCombat && !a_actor->IsInCombat()) {
    return 0.0f;
  }

  if (Settings::bUseAttackStaminaSystem) {
    auto flags       = atkData->data.flags;
    float weaponMass = 0.0f;
    if (a_actor->GetAttackingWeapon()) {
      weaponMass = a_actor->GetAttackingWeapon()->GetWeight();
    }
    if (Settings::bPowerAttackComsumeStaminaTweak &&
        (flags.any(RE::AttackData::AttackFlag::kPowerAttack) || flags.any(RE::AttackData::AttackFlag::kChargeAttack)))
      return Settings::fPowerAttackStaminaCostBase + Settings::fPowerAttackStaminaCostPerMass * weaponMass;
    else if (Settings::bNormalAttackComsumeStamina)
      return Settings::fNormalAttackStaminaCostBase + Settings::fNormalAttackStaminaCostPerMass * weaponMass;
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
  if (a_value < 0) {
    if (a_akValue == RE::ActorValue::kStamina) {
      if (Settings::bUseExhaustionSystem &&
          a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) + a_value <= 0) {
        Handler::Attack::GetSingleton().EnterExhaustion(a_actor);
      }
    }
    return a_value;
  }
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
    if (Settings::bUseExhaustionSystem && Handler::Attack::GetSingleton().IsInExhaustion(a_actor)) {
      if (a_actor->AsActorValueOwner()->GetActorValue(RE::ActorValue::kStamina) /
              (a_actor->AsActorValueOwner()->GetPermanentActorValue(RE::ActorValue::kStamina) +
               a_actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, RE::ActorValue::kStamina)) >=
          Settings::fExhaustionEndPercent)
        Handler::Attack::GetSingleton().QuitExhaustion(a_actor);
    }
    break;
  case RE::ActorValue::kHealth:
    break;
  }
  if (Settings::bEnablePoiseExecution) {
    if (!Settings::bRestorePoiseInCombat && a_actor->IsInCombat())
      return a_value;
    if (Handler::Execution::GetSingleton().IsVictim(a_actor))
      return a_value;
    Handler::Poise::RimPoise::GetSingleton().RestorePoiseHealth(
        a_actor, a_actor->IsInCombat() ? Settings::fRestorePoiseSpeed * Settings::fRestorePoiseSpeedCombatMult
                                       : Settings::fRestorePoiseSpeed);
  }
  return a_value;
}

void Hook_OnModActorValue::ModActorValue_NPC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier,
                                             RE::ActorValue a_akValue, float a_value)
{
  if (a_modifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
    REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
    RE::Actor* a_actor = &REL::RelocateMember<RE::Actor>(a_this, offset.offset());
    a_value            = ModActorValue(a_actor, a_akValue, a_value);
  }
  _ModActorValue_NPC(a_this, a_modifier, a_akValue, a_value);
}

void Hook_OnModActorValue::ModActorValue_PC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier,
                                            RE::ActorValue a_akValue, float a_value)
{
  if (a_modifier == RE::ACTOR_VALUE_MODIFIER::kDamage) {
    REL::VariantOffset offset(-0xB0, -0xB8, 0x0);
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

  if (Settings::bEnablePoiseExecution)
    Handler::Poise::RimPoise::GetSingleton().ProcessMeleeHit(aggressor, victim, hitData);

  if (Settings::bUseExhaustionSystem && Handler::Attack::GetSingleton().IsInExhaustion(aggressor)) {
    logger::info("GetAttackStaminaCost: Exhausted Actor {} Origin {}", aggressor->GetDisplayFullName(),
                 hitData.totalDamage);
    hitData.totalDamage *= Settings::fExhaustionDamageMult;
    logger::info("GetAttackStaminaCost: Exhausted Actor {} Modify {}", aggressor->GetDisplayFullName(),
                 hitData.totalDamage);
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