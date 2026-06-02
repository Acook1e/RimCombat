#pragma once

namespace Events
{
// 动画事件处理
class AnimEvent
{
public:
  static void Install()
  {
    logger::info("AnimEvent: Installing animation event hook...");
    REL::Relocation<uintptr_t> AnimEventVtbl_NPC{RE::VTABLE_Character[2]};
    REL::Relocation<uintptr_t> AnimEventVtbl_PC{RE::VTABLE_PlayerCharacter[2]};

    _ProcessEvent_NPC = AnimEventVtbl_NPC.write_vfunc(0x1, ProcessEvent_NPC);
    _ProcessEvent_PC  = AnimEventVtbl_PC.write_vfunc(0x1, ProcessEvent_PC);
  }

private:
  static inline bool ProcessEvent(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                                  RE::BSAnimationGraphEvent* event,
                                  RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource);

  static RE::BSEventNotifyControl
  ProcessEvent_NPC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                   RE::BSAnimationGraphEvent* event,
                   RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource);

  static RE::BSEventNotifyControl
  ProcessEvent_PC(RE::BSTEventSink<RE::BSAnimationGraphEvent>* sink,
                  RE::BSAnimationGraphEvent* event,
                  RE::BSTEventSource<RE::BSAnimationGraphEvent>* eventSource);

  static inline REL::Relocation<decltype(ProcessEvent_NPC)> _ProcessEvent_NPC;
  static inline REL::Relocation<decltype(ProcessEvent_PC)> _ProcessEvent_PC;
};

class HitEvent : public RE::BSTEventSink<RE::TESHitEvent>
{
public:
  static HitEvent* GetSingleton()
  {
    static HitEvent singleton;
    return &singleton;
  }
  static void Install()
  {
    auto source = RE::ScriptEventSourceHolder::GetSingleton()->GetEventSource<RE::TESHitEvent>();
    if (source)
      source->AddEventSink(GetSingleton());
    logger::info("HitEvent: Installing event hook");
  }

  RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent* event,
                                        RE::BSTEventSource<RE::TESHitEvent>* eventSource) override;
};

// 菜单事件监听
class MenuEvent : public RE::BSTEventSink<RE::MenuOpenCloseEvent>
{
public:
  static MenuEvent* GetSingleton()
  {
    static MenuEvent singleton;
    return &singleton;
  }
  static void Install()
  {
    auto ui = RE::UI::GetSingleton();
    if (ui)
      ui->AddEventSink<RE::MenuOpenCloseEvent>(GetSingleton());
    logger::info("MenuEvent: Installing event hook");
  }

  RE::BSEventNotifyControl
  ProcessEvent(const RE::MenuOpenCloseEvent* event,
               RE::BSTEventSource<RE::MenuOpenCloseEvent>* eventSource) override;
};

// Mod事件回调
class ModEvent : public RE::BSTEventSink<SKSE::ModCallbackEvent>
{
public:
  static ModEvent* GetSingleton()
  {
    static ModEvent singleton;
    return &singleton;
  }
  static void Install()
  {
    RegisterWeaponArtHUDInput();
    RegisterWeaponArtMenuInput();

    auto source = SKSE::GetModCallbackEventSource();
    if (source)
      source->AddEventSink(GetSingleton());
    logger::info("ModEvent: Installing event hook");
  }

  static void RegisterWeaponArtHUDInput();
  static void RegisterWeaponArtMenuInput();

  RE::BSEventNotifyControl
  ProcessEvent(const SKSE::ModCallbackEvent* event,
               RE::BSTEventSource<SKSE::ModCallbackEvent>* eventSource) override;

private:
  static inline std::int32_t id_WeaponArtHUD;
  static inline std::int32_t id_WeaponArtMenu;
};

inline void Install()
{
  AnimEvent::Install();
  HitEvent::Install();
  MenuEvent::Install();
  ModEvent::Install();
}
}  // namespace Events