#include "Block.h"

#include "Utils.h"

void Block::StartBlock(RE::Actor* a_actor)
{
  if (!a_actor)
    return;
  std::lock_guard lock(mutex);
  blockStartTimes[a_actor] = Utils::GetTime(std::chrono::milliseconds());
}

void Block::EndBlock(RE::Actor* a_actor)
{
  if (!a_actor)
    return;
  std::lock_guard lock(mutex);
  blockStartTimes.erase(a_actor);
  auto now = Utils::GetTime(std::chrono::milliseconds());
  if (lastCleanTime < now - 1000) {
    std::erase_if(blockStartTimes, [now](auto& pair) {
      // Remove Dead actors or block times more than timed block limit
      return pair.first->IsDead() || pair.second < now - 300;
    });
    lastCleanTime = now;
  }
}

bool Block::IsTimedBlock(RE::Actor* a_actor)
{
  if (!a_actor)
    return false;
  std::lock_guard lock(mutex);
  int64_t interval = 0;
  if (blockStartTimes.find(a_actor) != blockStartTimes.end())
    interval = Utils::GetTime(std::chrono::milliseconds()) - blockStartTimes[a_actor];
  logger::info("Block::IsTimedBlock: Actor: {}, Interval: {}.", a_actor->GetName(), interval);
  return interval > 0 && interval < 300;
}