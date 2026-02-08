#include "Event.h"
#include "Hooks.h"
#include "Menu.h"
#include "Posture.h"
#include "Settings.h"

void onPostLoad()
{
  Settings::SettingsDirCheck();
  Menu::GetSingleton();
  Posture::GetSingleton().InitHUD();
}

void onDataLoaded()
{
  Settings::LoadSettings();
  Hooks::Install();
  Events::Install();
}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
  switch (a_msg->type) {
  case SKSE::MessagingInterface::kPostLoad:
    onPostLoad();
    break;
  case SKSE::MessagingInterface::kPostPostLoad:
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