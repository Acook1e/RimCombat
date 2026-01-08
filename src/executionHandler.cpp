#include "executionHandler.h"

namespace Handler
{
void Execution::Update()
{
  logger::info("Handler::Execution::Update: Updating execution states.");
  if (!Settings::bEnablePoiseExecution)
    return;
  if (victimActors.empty())
    return;
  mtx.lock();
  logger::info("Handler::Execution::Update: Locked mutex for execution update.");
  auto now   = std::chrono::steady_clock::now();
  auto front = victimActors.begin();
  if (std::chrono::duration_cast<std::chrono::milliseconds>(now - front->startTime).count() >=
      static_cast<std::uint64_t>(Settings::fPoiseBreakStunTime * 1000)) {
    RE::Actor* actor = front->actorHandle.get().get();
    if (actor && !actor->IsDead()) {
      // SKSE::GetTaskInterface()->AddTask([actor]() {});
      Poise::RimPoise::GetSingleton().ResetPoiseHealth(actor);
      actor->SetGraphVariableBool(VictimMarkStr, false);
      logger::info("Handler::Execution::Update: Actor {} Execution Mark removed after stun time.",
                   actor->GetDisplayFullName());
    }
    victimActors.pop_front();
  }
  mtx.unlock();
}

void Execution::EnterExecutionState(RE::Actor* actor)
{
  if (!Settings::bEnablePoiseExecution || !actor)
    return;
  if (actor->IsDead())
    return;
  mtx.lock();
  actor->SetGraphVariableBool(VictimMarkStr, true);
  victimActors.push_back({actor->GetHandle(), std::chrono::steady_clock::now()});
  SKSE::GetTaskInterface()->AddTask([actor]() {
    actor->NotifyAnimationGraph("RimVictimEnter");
  });
  mtx.unlock();
}

RE::Actor* Execution::FindNearestExecutionTarget(RE::Actor* attacker)
{
  RE::Actor* victim = nullptr;
  float distance    = Settings::fExecutionMaxDistance;
  for (const auto& execData : victimActors) {
    RE::Actor* currentVictim = execData.actorHandle.get().get();
    if (!currentVictim || !currentVictim->Is3DLoaded() || currentVictim->IsDead() || currentVictim->IsInKillMove() ||
        !currentVictim->GetActorRuntimeData().currentProcess ||
        !currentVictim->GetActorRuntimeData().currentProcess->InHighProcess())
      continue;
    float currentDistance = attacker->GetPosition().GetDistance(currentVictim->GetPosition());
    if (currentDistance < distance) {
      distance = currentDistance;
      victim   = currentVictim;
    }
  }
  return victim;
}

void Execution::TryExecution(RE::Actor* attacker)
{
  // Condition Checks
  if (!Settings::bEnablePoiseExecution || !attacker)
    return;
  if (victimActors.empty())
    return;
  if (!Settings::bEnablePoiseExecution || !attacker || attacker->IsDead() || !attacker->Is3DLoaded() ||
      attacker->IsInKillMove() || attacker->IsOnMount())
    return;
  mtx.lock();
  RE ::Actor* victim = FindNearestExecutionTarget(attacker);
  if (!victim || victim->IsDead() || !victim->Is3DLoaded() || victim->IsInKillMove() || victim->IsOnMount() ||
      victim->AsMagicTarget()->HasEffectWithArchetype(RE::MagicTarget::Archetype::kParalysis)) {
    mtx.unlock();
    return;
  }

  logger::info("Handler::Execution: Actor {} is attempting to execute Actor {}.", attacker->GetDisplayFullName(),
               victim->GetDisplayFullName());

  // Reset victim poise and remove execution mark
  Poise::RimPoise::GetSingleton().ResetPoiseHealth(victim);
  victim->SetGraphVariableBool(VictimMarkStr, false);

  // Remove victim from the queue
  auto it = std::find_if(victimActors.begin(), victimActors.end(), [victim](const ExecutionData& data) {
    return data.actorHandle == victim->GetHandle();
  });
  if (it != victimActors.end())
    victimActors.erase(it);

  // Notify animation graph to play execution animation
  SKSE::GetTaskInterface()->AddTask([this, attacker, victim]() {
    attacker->NotifyAnimationGraph("attackStop");
    victim->NotifyAnimationGraph("RimVictimExecuted");
    // attacker->NotifyAnimationGraph("attackStart");
    // std::unique_ptr<RE::TESActionData> data(RE::TESActionData::Create());
    // data->source = RE::NiPointer<RE::TESObjectREFR>(attacker);
    // data->action = Settings::executorAction;
    // PerformAction(data.get());
  });
  mtx.unlock();
}
}  // namespace Handler