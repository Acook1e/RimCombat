#pragma once

#include "pch.h"

#include "executionHandler.h"
#include "hudHandler.h"

namespace Hooks
{
class Hook_OnGetAttackStaminaCost  // Actor__sub_140627930+16E	call ActorValueOwner__sub_1403BEC90
{
  /*to cancel out vanilla power attack stamina consumption.*/
public:
  static void install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook = REL::VariantID(37650, 38603, 0).address() + REL::VariantOffset(0x16E, 0x171, 0x0).offset();
    // SE:627930 + 16E => 3BEC90 AE:64D350 + 171 => 3D6720

    _getAttackStaminaCost = trampoline.write_call<5>(hook, getAttackStaminaCost);

    logger::info("Hooks: OnGetAttackStaminaCost installed.");
  }

private:
  static float getAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData);
  static inline REL::Relocation<decltype(getAttackStaminaCost)> _getAttackStaminaCost;
};

class Hook_OnCheckStaminaRegenCondition  // block stamina regen during attack
{
public:
  static void install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook = REL::VariantID(37510, 38452, 0).address() + REL::VariantOffset(0x62, 0x6F, 0x0).offset();
    // 140620690       140645AA0

    _HasFlags1 = trampoline.write_call<5>(hook, HasFlags1);
    logger::info("Hooks: CheckStaminaRegenCondition installed.");
  }

private:
  static bool HasFlags1(RE::ActorState* a_this, uint16_t a_flags);
  static inline REL::Relocation<decltype(HasFlags1)> _HasFlags1;
  // 14063C330       140662930
};

class Hook_OnModActorValue
{
public:
  static void install()
  {
    // auto& trampoline = SKSE::GetTrampoline();
    // SKSE::AllocTrampoline(14);

    // std::uintptr_t hook = REL::VariantID(37510, 38452, 0).address() + REL::VariantOffset(0x176, 0xE1, 0x0).offset();
    // // 140620900		140645d30
    // _RestoreActorValue = trampoline.write_call<5>(hook, RestoreActorValue);

    REL::Relocation<uintptr_t> vtbl_NPC{RE::VTABLE_Character[5]};
    REL::Relocation<uintptr_t> vtbl_PC{RE::VTABLE_PlayerCharacter[5]};

    _ModActorValue_NPC = vtbl_NPC.write_vfunc(0x06, ModActorValue_NPC);
    _ModActorValue_PC  = vtbl_PC.write_vfunc(0x06, ModActorValue_PC);
    logger::info("Hooks: OnRestoreActorValue installed.");
  }

private:
  static float ModActorValue(RE::Actor* a_actor, RE::ActorValue a_akValue, float a_value);

  static void ModActorValue_NPC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier,
                                RE::ActorValue a_akValue, float a_value);

  static void ModActorValue_PC(RE::ActorValueOwner* a_this, RE::ACTOR_VALUE_MODIFIER a_modifier,
                               RE::ActorValue a_akValue, float a_value);

  static inline REL::Relocation<decltype(ModActorValue_NPC)> _ModActorValue_NPC;
  static inline REL::Relocation<decltype(ModActorValue_PC)> _ModActorValue_PC;
  // 140620900		140645d30
};

class Hook_OnMeleeHit
{
public:
  static void install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook = REL::VariantID(37673, 38627, 0).address() + REL::VariantOffset(0x3C0, 0x4A8, 0x0).offset();
    // 140628C20       14064E760
    _ProcessHit = trampoline.write_call<5>(hook, processHit);
    logger::info("Hooks: OnMeleeHit installed.");
  }

private:
  static void processHit(RE::Actor* victim, RE::HitData& hitData);
  static inline REL::Relocation<decltype(processHit)> _ProcessHit;
  // 140626400       14064BAB0
};

class Hook_OnAttackAction
{
public:
  static void install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook  = REL::VariantID(48139, 49170, 0).address() + REL::VariantOffset(0x4D7, 0x435, 0x0).offset();
    _PerformAttackAction = trampoline.write_call<5>(hook, PerformAttackAction);
    logger::info("Hooks: OnAttackAction installed.");
  }

private:
  static bool PerformAttackAction(RE::TESActionData* a_actionData);
  static inline REL::Relocation<decltype(PerformAttackAction)> _PerformAttackAction;
};

class Hook_AttackBlockHandler_OnProcessButton  // not used
{
public:
  static void install()
  {
    REL::Relocation<std::uintptr_t> atkbckVtbl{RE::VTABLE_AttackBlockHandler[0]};

    _ProcessButton = atkbckVtbl.write_vfunc(0x4, ProcessButton);
    logger::info("Hooks: AttackBlockHandler_OnProcessButton installed.");
  }

private:
  static void ProcessButton(RE::AttackBlockHandler* a_this, RE::ButtonEvent* a_event, RE::PlayerControlsData* a_data);
  static inline REL::Relocation<decltype(ProcessButton)> _ProcessButton;
};

class Hook_OnPlayerUpdate  // no longer used
{
public:
  static void install()
  {
    REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{RE::VTABLE_PlayerCharacter[0]};

    _Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
    logger::info("Hooks: OnPlayerUpdate installed.");
  }

private:
  static void Update(RE::PlayerCharacter* a_this, float a_delta)
  {
    Handler::Execution::GetSingleton().Update();
    _Update(a_this, a_delta);
  }
  static inline REL::Relocation<decltype(Update)> _Update;
};

inline void InstallHooks()
{
  Hook_OnGetAttackStaminaCost::install();
  Hook_OnCheckStaminaRegenCondition::install();
  Hook_OnModActorValue::install();
  Hook_OnMeleeHit::install();
  Hook_OnAttackAction::install();
  // Hook_AttackBlockHandler_OnProcessButton::install();

  Hook_OnPlayerUpdate::install();
}
}  // namespace Hooks