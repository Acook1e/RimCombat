#include "pch.h"

#include "event.h"
#include "hook.h"
#include "hudHandler.h"
#include "menu.h"

void onDataLoaded()
{
  Hooks::InstallHooks();
  Settings::InitSettings();
  Events::Register();
}

void onPostLoad()
{
  Handler::HUD::GetSingleton().InitHUD();
}

void onPostLoadGame()
{
  if (Settings::bUsePoiseHUD)
    Handler::HUD::GetSingleton().RequestHUD();
}

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

  const auto plugin{SKSE::PluginDeclaration::GetSingleton()};
  const auto version{plugin->GetVersion()};

  logger::info("Runtime version: {}", skse->RuntimeVersion());

  auto messaging = SKSE::GetMessagingInterface();
  if (!messaging->RegisterListener("SKSE", MessageHandler)) {
    return false;
  }

  Menu::Register();

  return true;
}