#pragma once

#include "pch.h"

#include "executionHandler.h"
#include "hudHandler.h"

namespace Hooks
{
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

inline void InstallHooks()
{
  Hook_OnAttackAction::install();
  // Hook_AttackBlockHandler_OnProcessButton::install();
}
}  // namespace Hooks