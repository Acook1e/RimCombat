#include "Combat/Exhausted.h"
#include "Combat/Posture.h"
#include "Combat/Weapon.h"
#include "Combat/WeaponArt.h"
#include "Core/Event.h"
#include "Core/Hooks.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "GUI/Menu.h"
#include "GUI/UI.h"

// 内部数据初始化与序列化注册
void onPostLoad()
{
  Settings::LoadSettings();
  WeaponArt::Manager::GetSingleton();
  WeaponArt::PlayerStat::GetSingleton();  // 必须在Manager之后
}

// 依赖外部API的初始化必须在PostLoad之后进行
// 以确保API已准备就绪
void onPostPostLoad()
{
  // 外部API
  UI::Initialize();
  UI::TrueHUD::GetSingleton();
  UI::WeaponArtMenu::GetSingleton();
  Menu::GetSingleton();

  // 依赖外部API的系统
  Posture::GetSingleton();
  Exhausted::GetSingleton();
}

void onDataLoaded()
{
  Settings::UpdateGameSettings();
  Hooks::Install();
  Events::Install();
  Weapon::Initialize();
}

void MessageHandler(SKSE::MessagingInterface::Message* msg)
{
  switch (msg->type) {
  case SKSE::MessagingInterface::kPostLoad:
    onPostLoad();
    break;
  case SKSE::MessagingInterface::kPostPostLoad:
    onPostPostLoad();
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
  SKSE::Init(skse, true);

  logger::info("Runtime version: {}", skse->RuntimeVersion());

  auto messaging = SKSE::GetMessagingInterface();
  if (!messaging->RegisterListener("SKSE", MessageHandler)) {
    return false;
  }

  return true;
}