#include "Combat/Block.h"
#include "Combat/Execution.h"
#include "Combat/Exhausted.h"
#include "Combat/Posture.h"
#include "Combat/Weapon.h"
#include "Combat/WeaponArt.h"
#include "Core/Event.h"
#include "Core/Hooks.h"
#include "Core/Serialization.h"
#include "Core/Settings.h"
#include "GUI/Localization.h"
#include "GUI/Menu.h"
#include "GUI/UI.h"

// 内部数据初始化与序列化注册
void onPostLoad()
{
  Settings::LoadSettings();
  Serialization::Initialize();
  Localization::Initialize();
  WeaponArt::Manager::GetSingleton();
  WeaponArt::PlayerStat::GetSingleton();  // 必须在Manager之后
  Block::GetSingleton();
  Posture::GetSingleton();
  Exhausted::GetSingleton();
  Execution::GetSingleton();
}

// 依赖外部API的初始化必须在PostLoad之后进行
// 以确保API已准备就绪
void onPostPostLoad()
{
  // 外部API
  UI::Initialize();
  Menu::GetSingleton();

  // 依赖外部API的系统
  UI::TrueHUD::GetSingleton();
  UI::WeaponArtMenu::GetSingleton();
  UI::WeaponArtHUD::GetSingleton();
}

// 依赖游戏数据的初始化必须在DataLoaded之后进行
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

// TODO: 反击系统（触发条件/受击帧/伤害倍率）
// 反击系统搁置，风格不合模组理念，过于强力且难以平衡

// TODO: 限时格挡（特效）
// TODO: 战技分配（NPC战技系统）
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