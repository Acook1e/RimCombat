#include "Event.h"
#include "Hooks.h"
#include "Menu.h"
#include "Posture.h"
#include "Serialization.h"
#include "Settings.h"
#include "WeaponArt.h"

void onPostLoad()
{
  Settings::LoadSettings();
  Menu::GetSingleton();
  Posture::GetSingleton().InitHUD();
}

void onDataLoaded()
{
  Settings::UpdateGameSettings();
  Hooks::Install();
  Events::Install();
  // WeaponArt::GetSingleton().Init();
  WeaponArt::InfoCard::Register();
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
  switch (a_msg->type) {
  case SKSE::MessagingInterface::kPostLoad:
    onPostLoad();
    break;
  case SKSE::MessagingInterface::kDataLoaded:
    onDataLoaded();
    break;
  case SKSE::MessagingInterface::kNewGame:
    break;
  case SKSE::MessagingInterface::kPreLoadGame:
    break;
  case SKSE::MessagingInterface::kPostLoadGame:
    break;
  }
}

// TODO: 限时格挡（特效）
// TODO: 反击系统（触发条件/受击帧/伤害倍率）
// TODO: 战技分配（资源消耗/解锁/热键/冲突处理）
// TODO: 架势崩溃（阈值/恢复/动画与AI反应）
// TODO: 处决系统（目标状态/镜头/安全锁定）

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
  SKSE::Init(skse);

  logger::info("Runtime version: {}", skse->RuntimeVersion());

  auto messaging = SKSE::GetMessagingInterface();
  if (!messaging->RegisterListener("SKSE", MessageHandler)) {
    return false;
  }

  return true;
}