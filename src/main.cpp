#include "Event.h"
#include "Hooks.h"
#include "Menu.h"
#include "Posture.h"
#include "Settings.h"

void onDataLoaded()
{
  Settings::LoadSettings();
  Hooks::Install();
  Events::Install();
}

void onPostLoad()
{
  Posture::GetSingleton().InitHUD();
}

void onPostLoadGame() {}

void MessageHandler(SKSE::MessagingInterface::Message* a_msg)
{
  switch (a_msg->type) {
  case SKSE::MessagingInterface::kDataLoaded:
    onDataLoaded();
    break;
  case SKSE::MessagingInterface::kPostLoad:
    onPostLoad();
    break;
  case SKSE::MessagingInterface::kPostLoadGame:
    onPostLoadGame();
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

  Menu::GetSingleton();

  return true;
}