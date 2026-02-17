#include "Utils.h"

namespace Utils
{
float GetCurrentMaxActorValue(RE::Actor* actor, RE::ActorValue av)
{
  return actor->AsActorValueOwner()->GetPermanentActorValue(av) + actor->GetActorValueModifier(RE::ACTOR_VALUE_MODIFIER::kTemporary, av);
}

RE::FormID GetSelectedItem()
{
  RE::GFxValue Menu_mc;
  auto ui                = RE::UI::GetSingleton();
  auto menu              = ui ? ui->GetMenu(RE::InventoryMenu::MENU_NAME) : nullptr;
  RE::GFxMovieView* view = menu ? menu->uiMovie.get() : nullptr;
  if (!view)
    return 0;
  if (!view->GetVariable(&Menu_mc, "_root.Menu_mc"))
    return 0;

  RE::GFxValue inventoryLists;
  RE::GFxValue itemList;
  RE::GFxValue selectedEntry;
  RE::GFxValue formID;
  if (!Menu_mc.GetMember("inventoryLists", &inventoryLists) || !inventoryLists.GetMember("itemList", &itemList) ||
      !itemList.GetMember("selectedEntry", &selectedEntry) || !selectedEntry.IsObject() || !selectedEntry.GetMember("formId", &formID) ||
      formID.IsNull())
    return 0;
  return static_cast<RE::FormID>(formID.GetUInt());
}

void MenuLogger::LogMessageVarg(LogMessageType, const char* a_fmt, std::va_list a_argList)
{
  std::string fmt(a_fmt ? a_fmt : "");
  while (!fmt.empty() && fmt.back() == '\n') {
    fmt.pop_back();
  }

  std::va_list args;
  va_copy(args, a_argList);
  std::vector<char> buf(static_cast<std::size_t>(std::vsnprintf(0, 0, fmt.c_str(), a_argList) + 1));
  std::vsnprintf(buf.data(), buf.size(), fmt.c_str(), args);
  va_end(args);

  logger::info("{}", buf.data());
}
}  // namespace Utils