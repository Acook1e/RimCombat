#include "Combat/Block.h"

#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "Utils.h"

Block::Block()
{
  Serialization::RegisterRevertCallback(serialType, [](SKSE::SerializationInterface*) {
    std::lock_guard lock(mtx);
    blockStartTimes.clear();
  });
}

void Block::Update()
{
  std::lock_guard lock(mtx);
  if (blockStartTimes.empty())
    return;

  // 定期清理过期的格挡时间记录，避免map无限增长
  auto now = Utils::GetTime<std::chrono::milliseconds>();
  for (auto it = blockStartTimes.begin(); it != blockStartTimes.end();) {
    if (now - it->second > Settings::uTimedBlockLimit)
      it = blockStartTimes.erase(it);
    else
      ++it;
  }
}

void Block::StartBlock(RE::Actor* actor)
{
  if (!actor)
    return;
  std::lock_guard lock(mtx);
  blockStartTimes[actor] = Utils::GetTime<std::chrono::milliseconds>();
}

void Block::EndBlock(RE::Actor* actor)
{
  if (!actor)
    return;
  std::lock_guard lock(mtx);
  blockStartTimes.erase(actor);
}

bool Block::IsTimedBlock(RE::Actor* actor)
{
  if (!actor || !Settings::bTimedBlockEnabled)
    return false;

  std::lock_guard<std::mutex> lock(mtx);
  // 如果没有格挡记录，说明不是限时格挡
  if (blockStartTimes.find(actor) == blockStartTimes.end())
    return false;

  // 每次格挡只能触发一次限时格挡，所以这里直接删除记录
  auto startTime = blockStartTimes[actor];
  blockStartTimes.erase(actor);

  auto now = Utils::GetTime<std::chrono::milliseconds>();
  if (now - startTime < Settings::uTimedBlockLimit) {
    logger::info("Timed Block: Actor {} performed a timed block!", actor->GetDisplayFullName());
    for (auto& callback : timeBlockCallbacks)
      callback(actor);
    return true;
  }
  return false;
}

void Block::AddTimeBlockListener(std::function<void(RE::Actor*)> callback)
{
  std::lock_guard lock(mtx);
  timeBlockCallbacks.push_back(callback);
}