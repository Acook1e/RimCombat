#pragma once

#include "pch.h"

#include "poiseHandler.h"
#include "setting.h"

namespace Handler
{
class Execution
{
public:
  static Execution& GetSingleton()
  {
    static Execution singleton;
    return singleton;
  }

  void Update();

  bool IsVictim(RE::Actor* actor)
  {
    bool res = false;
    actor->GetGraphVariableBool(VictimMarkStr, res);
    return res;
  }

  void EnterExecutionState(RE::Actor* actor);

  RE::Actor* FindNearestExecutionTarget(RE::Actor* attacker);

  void TryExecution(RE::Actor* attacker);

private:
  struct ExecutionData
  {
    RE::ActorHandle actorHandle;
    std::chrono::steady_clock::time_point startTime;
  };

  typedef bool PerformAction_t(RE::TESActionData*);
  REL::Relocation<PerformAction_t> PerformAction{REL::VariantID(40551, 41557, 0x0)};

  constexpr static std::string_view VictimMarkStr = "bRimIsVictim";

  std::deque<ExecutionData> victimActors;
  std::mutex mtx;
};
}  // namespace Handler
