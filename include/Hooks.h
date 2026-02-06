#pragma once

#include "Settings.h"

namespace Hooks
{
class Hook_OnGetAttackStaminaCost  // Actor__sub_140627930+16E	call ActorValueOwner__sub_1403BEC90
{
  /*to cancel out vanilla power attack stamina consumption.*/
public:
  static void Install()
  {
    auto& trampoline = SKSE::GetTrampoline();
    SKSE::AllocTrampoline(14);

    std::uintptr_t hook = REL::VariantID(37650, 38603, 0).address() + REL::VariantOffset(0x16E, 0x171, 0x0).offset();
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

    std::uintptr_t hook = REL::VariantID(37673, 38627, 0).address() + REL::VariantOffset(0x3C0, 0x4A8, 0x0).offset();
    // 140628C20       14064E760
    _ProcessHit = trampoline.write_call<5>(hook, ProcessHit);
    logger::info("Hook: OnMeleeHit installed.");
  }

private:
  static void ProcessHit(RE::Actor* victim, RE::HitData& hitData);
  static inline REL::Relocation<decltype(ProcessHit)> _ProcessHit;
  // 140626400       14064BAB0
};
inline void Install()
{
  Hook_OnGetAttackStaminaCost::Install();
  Hook_OnMeleeHit::Install();
}
}  // namespace Hooks