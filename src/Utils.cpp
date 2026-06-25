#include "Utils.h"

namespace Utils
{
// 字符串工具
std::string join(std::vector<std::string>& vec, char delimiter)
{
  return std::views::join_with(vec, delimiter) | std::ranges::to<std::string>();
}

std::vector<std::string> split(const std::string& str, char delimiter)
{
  return std::views::split(str, delimiter) | std::views::transform([](auto&& part) {
           return std::string(part.begin(), part.end());
         }) |
         std::ranges::to<std::vector>();
}

std::optional<float> toFloat(const std::string& str)
{
  try {
    return std::stof(str);
  } catch (const std::exception& e) {
    logger::error("Failed to parse float from string '{}': {}", str, e.what());
    return std::nullopt;
  }
}

std::optional<std::int32_t> toInt(const std::string& str, int base)
{
  try {
    return std::stoi(str, nullptr, base);
  } catch (const std::exception& e) {
    logger::error("Failed to parse int from string '{}': {}", str, e.what());
    return std::nullopt;
  }
}

// 游戏相关工具
float GetCurrentMaxActorValue(RE::Actor* actor, RE::ActorValue av)
{
  return actor->AsActorValueOwner()->GetPermanentActorValue(av) +
         actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
}
float GetCurrentActorValuePercent(RE::Actor* actor, RE::ActorValue av)
{
  float current = actor->AsActorValueOwner()->GetActorValue(av);
  float max     = GetCurrentMaxActorValue(actor, av);
  if (max == 0.0f)
    return 0.0f;
  return current / max;
}
RE::InventoryEntryData* GetSelectedItemEntry()
{
  auto ui   = RE::UI::GetSingleton();
  auto menu = ui ? ui->GetMenu<RE::InventoryMenu>(RE::InventoryMenu::MENU_NAME) : nullptr;
  if (!menu)
    return nullptr;
  if (RE::GFxValue itemIndex; menu->uiMovie->GetVariable(
          &itemIndex, "_root.Menu_mc.inventoryLists.itemList.selectedEntry.itemIndex")) {
    auto items = menu->itemList->items;
    if (items.empty())
      return nullptr;
    auto idx = static_cast<std::int32_t>(itemIndex.GetNumber());
    if (idx < 0 || static_cast<std::size_t>(idx) >= items.size())
      return nullptr;
    return items[idx]->data.objDesc;
  }
  return nullptr;
}
void PlaySFX(RE::Actor* actor, RE::BGSSoundDescriptorForm* descriptor, RE::NiPoint3 position,
             float volume)
{
  if (!actor || !descriptor)
    return;

  auto* audioManager = RE::BSAudioManager::GetSingleton();
  if (!audioManager)
    return;

  RE::BSSoundHandle handle;
  audioManager->GetSoundHandle(handle, descriptor);
  if (!handle.IsValid())
    return;
  handle.SetPosition(position);
  handle.SetVolume(volume);
  handle.SetObjectToFollow(actor->Get3D());
  handle.Play();
}
void KnockExplosion(RE::AIProcess* process, RE::Actor* actor, const RE::NiPoint3& location,
                    float magnitude)
{
  using func_t = decltype(&KnockExplosion);
  static REL::Relocation<func_t> func{RELOCATION_ID(38858, 39895)};

  if (!process || !actor)
    return;

  return func(process, actor, location, magnitude);
}

// 主线程相关
std::mutex taskMutex;
std::deque<std::function<void()>> mainThreadTasks;

void AddTask(std::function<void()> task)
{
  std::lock_guard<std::mutex> lock(taskMutex);
  mainThreadTasks.push_back(std::move(task));
}
void MainUpdate()
{
  std::lock_guard<std::mutex> lock(taskMutex);
  if (mainThreadTasks.empty())
    return;

  for (auto& task : mainThreadTasks)
    task();
  mainThreadTasks.clear();
}
}  // namespace Utils