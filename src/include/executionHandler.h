#pragma once

#include "pch.h"

namespace Handler::ExecutionHandler
{
void EnterExecutionState(RE::Actor* actor)
{
  if (!Settings::bEnableExecution || !actor || !Settings::victimIdle)
    return;
  actor->GetActorRuntimeData().currentProcess->PlayIdle(actor, Settings::victimIdle, nullptr);
}

void QuitExecutionState(RE::Actor* actor)
{
  if (!Settings::bEnableExecution || !actor)
    return;
}
}  // namespace Handler::ExecutionHandler