#include "Combat/Block.h"

#include "Core/Settings.h"
#include "Utils.h"

void Block::StartBlock(RE::Actor* actor)
{
  if (!actor)
    return;
  std::lock_guard lock(mutex);
  blockStartTimes[actor] = Utils::GetTime(std::chrono::milliseconds());
}

void Block::EndBlock(RE::Actor* actor)
{
  if (!actor)
    return;
  std::lock_guard lock(mutex);
  blockStartTimes.erase(actor);
  auto now = Utils::GetTime(std::chrono::milliseconds());
  // Clean up old block times every second to prevent the map from growing
  // indefinitely, which may cause performance issues.
  if (lastCleanTime < now - 1000) {
    std::erase_if(blockStartTimes, [now](auto& pair) {
      // Remove Dead actors or block times more than timed block limit
      return pair.first->IsDead() ||
             pair.second < now - Settings::iTimedBlockLimit;
    });
    lastCleanTime = now;
  }
}

bool Block::IsTimedBlock(RE::Actor* actor)
{
  if (!actor)
    return false;
  std::lock_guard lock(mutex);
  int64_t interval = 0;
  if (blockStartTimes.find(actor) != blockStartTimes.end())
    interval =
        Utils::GetTime(std::chrono::milliseconds()) - blockStartTimes[actor];
  logger::info("Block::IsTimedBlock: Actor: {}, Interval: {}.",
               actor->GetName(), interval);
  if (interval > 0 && interval < Settings::iTimedBlockLimit) {
    // Timed block only trigger once and cold down at next block start, so we
    // set the start time to a past time to avoid repeated trigger.
    blockStartTimes[actor] -= 100;
    return true;
  }
  return false;
}