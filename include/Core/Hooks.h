#pragma once

#include "Settings.h"

namespace Hooks
{
class Hook_OnMainUpdate
{
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    REL::Relocation addr{REL::VariantID(35565, 36564, 0x5BAB10),
                         REL::VariantOffset(0x748, 0xc26, 0x7ee)};

    _MainUpdate = trampoline.write_call<5>(addr.address(), MainUpdate);
    logger::info("Hooks: OnMainUpdate installed.");
  }

private:
  static void MainUpdate();
  static inline REL::Relocation<decltype(MainUpdate)> _MainUpdate;
};

class Hook_OnActorUpdate
{
public:
  static void Install()
  {
    REL::Relocation<uintptr_t> vtbl_NPC{RE::VTABLE_Character[0]};
    REL::Relocation<uintptr_t> vtbl_PC{RE::VTABLE_PlayerCharacter[0]};

    _Update_NPC = vtbl_NPC.write_vfunc(0xAD, Update_NPC);
    _Update_PC  = vtbl_PC.write_vfunc(0xAD, Update_PC);
    logger::info("Hooks: OnActorUpdate installed.");
  }

private:
  static void Update_NPC(RE::Character* character, float delta) { _Update_NPC(character, delta); }
  static void Update_PC(RE::PlayerCharacter* player, float delta) { _Update_PC(player, delta); }

  static inline REL::Relocation<decltype(Update_NPC)> _Update_NPC;
  static inline REL::Relocation<decltype(Update_PC)> _Update_PC;
};
class Hook_OnGetAttackStaminaCost  // Actor__sub_140627930+16E	call
                                   // ActorValueOwner__sub_1403BEC90
{
  /*to cancel out vanilla power attack stamina consumption.*/
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook =
        REL::VariantID(37650, 38603, 0).address() + REL::VariantOffset(0x16E, 0x171, 0x0).offset();
    // SE:627930 + 16E => 3BEC90 AE:64D350 + 171 => 3D6720

    _GetAttackStaminaCost = trampoline.write_call<5>(hook, GetAttackStaminaCost);

    logger::info("Hooks: OnGetAttackStaminaCost installed.");
  }

private:
  static float GetAttackStaminaCost(RE::ActorValueOwner* avOwner, RE::BGSAttackData* atkData);
  static inline REL::Relocation<decltype(GetAttackStaminaCost)> _GetAttackStaminaCost;
};

class Hook_OnMeleeHit
{
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook =
        REL::VariantID(37673, 38627, 0).address() + REL::VariantOffset(0x3C0, 0x4A8, 0x0).offset();
    // 140628C20       14064E760
    _ProcessHit = trampoline.write_call<5>(hook, ProcessHit);
    logger::info("Hook: OnMeleeHit installed.");
  }

private:
  static void ProcessHit(RE::Actor* victim, RE::HitData& hitData);
  static inline REL::Relocation<decltype(ProcessHit)> _ProcessHit;
  // 140626400       14064BAB0
};

class Hook_OnAttackAction
{
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook =
        REL::VariantID(48139, 49170, 0).address() + REL::VariantOffset(0x4D7, 0xd35, 0x0).offset();
    _PerformAttackAction = trampoline.write_call<5>(hook, PerformAttackAction);
    logger::info("Hooks: OnAttackAction installed.");
  }

private:
  static bool PerformAttackAction(RE::TESActionData* a_actionData);
  static inline REL::Relocation<decltype(PerformAttackAction)> _PerformAttackAction;
};

class Hook_OnModActorValue
{
public:
  static void Install()
  {
    REL::Relocation<uintptr_t> vtbl_NPC{RE::VTABLE_Character[5]};
    REL::Relocation<uintptr_t> vtbl_PC{RE::VTABLE_PlayerCharacter[5]};

    _ModActorValue_NPC = vtbl_NPC.write_vfunc(0x06, ModActorValue_NPC);
    _ModActorValue_PC  = vtbl_PC.write_vfunc(0x06, ModActorValue_PC);
    logger::info("Hooks: OnModActorValue installed.");
  }

private:
  static void ModActorValue_NPC(RE::ActorValueOwner* avOwner, RE::ACTOR_VALUE_MODIFIER modifier,
                                RE::ActorValue akValue, float value);
  static void ModActorValue_PC(RE::ActorValueOwner* avOwner, RE::ACTOR_VALUE_MODIFIER modifier,
                               RE::ActorValue akValue, float value);
  static float ModActorValue(RE::Actor* actor, RE::ACTOR_VALUE_MODIFIER modifier,
                             RE::ActorValue akValue, float value);
  static float ModMaxActorValue(RE::Actor* actor, RE::ActorValue akValue, float value);
  static float ModCurrentActorValue(RE::Actor* actor, RE::ActorValue akValue, float value);

  static inline REL::Relocation<decltype(ModActorValue_NPC)> _ModActorValue_NPC;
  static inline REL::Relocation<decltype(ModActorValue_PC)> _ModActorValue_PC;
};

class Hook_OnEquipObject
{
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook =
        REL::VariantID(37938, 38894, 0).address() + REL::VariantOffset(0xE5, 0x170, 0x0).offset();

    _OnEquipObject = trampoline.write_call<5>(hook, OnEquipObject);
    logger::info("Hooks: OnEquipObject installed.");
  }

private:
  static void OnEquipObject(RE::ActorEquipManager* manager, RE::Actor* actor,
                            RE::TESBoundObject* object, std::uint64_t unk);

  static inline REL::Relocation<decltype(OnEquipObject)> _OnEquipObject;
};

class Hook_OnUnequipObject
{
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook =
        REL::VariantID(37945, 38901, 0).address() + REL::VariantOffset(0x138, 0x1B9, 0x0).offset();

    _OnUnequipObject = trampoline.write_call<5>(hook, OnUnequipObject);
    logger::info("Hooks: OnUnequipObject installed.");
  }

private:
  static void OnUnequipObject(RE::ActorEquipManager* manager, RE::Actor* actor,
                              RE::TESBoundObject* object, std::uint64_t unk);

  static inline REL::Relocation<decltype(OnUnequipObject)> _OnUnequipObject;
};

inline void Install()
{
  Hook_OnMainUpdate::Install();
  Hook_OnActorUpdate::Install();
  Hook_OnGetAttackStaminaCost::Install();
  Hook_OnMeleeHit::Install();
  Hook_OnAttackAction::Install();
  Hook_OnModActorValue::Install();
  Hook_OnEquipObject::Install();
  Hook_OnUnequipObject::Install();
}
}  // namespace Hooks