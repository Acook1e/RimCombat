#include "Event.h"
#include "Hooks.h"
#include "Menu.h"
#include "Posture.h"
#include "Serialization.h"
#include "Settings.h"

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

// TODO：限时格挡
// TODO: 反击系统
// TODO: 战技分配
// TODO：架势崩溃
// TODO: 处决系统

SKSEPluginLoad(const SKSE::LoadInterface* skse)
{
  SKSE::Init(skse);

  logger::info("Runtime version: {}", skse->RuntimeVersion());

  if (auto serialization = SKSE::GetSerializationInterface(); serialization) {
    Serialization::GetSingleton().Init(serialization);
  } else {
    logger::error("Serialization interface not available.");
  }

  auto messaging = SKSE::GetMessagingInterface();
  if (!messaging->RegisterListener("SKSE", MessageHandler)) {
    return false;
  }

  return true;
}