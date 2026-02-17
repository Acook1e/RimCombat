#include "WeaponArt.h"

#include "Utils.h"

#include "SimpleIni.h"

namespace WeaponArt
{
std::string_view ToString(WeaponArtType type)
{
  static std::unordered_map<WeaponArtType, std::string_view> typeToString = {
      {WeaponArtType::kOnce, "Once"}, {WeaponArtType::kStage, "Stage"}, {WeaponArtType::kStance, "Stance"}};
  auto it = typeToString.find(type);
  if (it != typeToString.end())
    return it->second;
  return "Unknown";
}

std::string_view ToString(WeaponArtRare rare)
{
  static std::unordered_map<WeaponArtRare, std::string_view> rareToString = {
      {WeaponArtRare::kCommon, "Common"}, {WeaponArtRare::kRare, "Rare"}, {WeaponArtRare::kEpic, "Epic"}, {WeaponArtRare::kLegendary, "Legendary"}};
  auto it = rareToString.find(rare);
  if (it != rareToString.end())
    return it->second;
  return "Unknown";
}

void Manager::Init()
{
  // Register serialization callbacks
  const SKSE::SerializationInterface* intfc = SKSE::GetSerializationInterface();
  if (intfc) {
    intfc->SetUniqueID(nexusID);
    intfc->SetSaveCallback([](SKSE::SerializationInterface* intfc) {
      GetSingleton().Save(intfc);
    });
    intfc->SetLoadCallback([](SKSE::SerializationInterface* intfc) {
      GetSingleton().Load(intfc);
    });
    intfc->SetRevertCallback([](SKSE::SerializationInterface* intfc) {
      GetSingleton().Revert(intfc);
    });
  }

  // Load weapon art info from ini files

  std::string weaponArtDir   = Settings::SettingsDir + "WeaponArt/";
  auto loadWeaponArtFromFile = [this](const std::filesystem::path& filePath) {
    std::string fileName = filePath.stem().string();
    CSimpleIniA ini;
    SI_Error rc = ini.LoadFile(filePath.string().data());
    if (rc < 0) {
      logger::error("Failed to load weapon art file: {}", filePath.string());
      return;
    }

    constexpr std::string_view section = "ModInfo";
    CSimpleIniA::TNamesDepend sections;
    ini.GetAllSections(sections);
    std::string modName = ini.GetValue(section.data(), "name", "null");

    auto modFile = RE::TESDataHandler::GetSingleton()->LookupLoadedModByName(modName);
    modFile      = modFile ? modFile : RE::TESDataHandler::GetSingleton()->LookupLoadedLightModByName(modName);
    if (!modFile) {
      logger::error("Mod {} not found for weapon art in file: {}", modName, filePath.string());
      return;
    }

    uint32_t prefix = 0;
    if (modFile->compileIndex == 0xFE) {
      prefix = modFile->compileIndex << 24;
      prefix |= modFile->smallFileCompileIndex << 12;
    } else if (modFile->compileIndex < 0xFE) {
      prefix = modFile->compileIndex << 24;
    } else {
      logger::error("Mod {} has invalid compile index for weapon art in file: {}", modName, filePath.string());
      return;
    }
    std::stringstream ss;
    ss << std::hex << prefix;
    logger::info("{} prefix {}", modName, ss.str());
    Settings::AddHashMapping(Utils::hash(modName.data(), modName.size()), prefix);

    for (const auto& section : sections) {
      if (std::string_view(section.pItem) == "ModInfo")
        continue;
      auto suffix            = static_cast<uint32_t>(std::stoul(ini.GetValue(section.pItem, "FormID", "0x0")));
      RE::SpellItem* artForm = RE::TESForm::LookupByID<RE::SpellItem>((prefix | suffix));
      if (!artForm) {
        logger::error("Failed to find form for weapon art in file: {}, formID: 0x{:X}", filePath.string(), (prefix | suffix));
        continue;
      }

      WeaponArtInfo artInfo;
      artInfo.artType           = static_cast<WeaponArtType>(ini.GetLongValue(section.pItem, "artType", 0));
      artInfo.artRare           = static_cast<WeaponArtRare>(ini.GetLongValue(section.pItem, "artRare", 0));
      artInfo.minPlayerLevel    = static_cast<uint8_t>(ini.GetLongValue(section.pItem, "minPlayerLevel", 0));
      artInfo.ignoreStagger     = ini.GetBoolValue(section.pItem, "ignoreStagger", false);
      artInfo.postureDamageMult = static_cast<uint8_t>(ini.GetLongValue(section.pItem, "postureDamageMult", 0));

      uint64_t persistForm = Settings::toPersistForm(Utils::hash(modName.data(), modName.size()), suffix);
      infoMap[persistForm] = artInfo;
    }
  };

  std::error_code ec;
  if (!std::filesystem::exists(weaponArtDir, ec)) {
    logger::info("WeaponArt directory does not exist, skipping loading weapon arts.");
    return;
  }
  for (const auto& entry : std::filesystem::directory_iterator(weaponArtDir, ec)) {
    if (entry.is_regular_file() && entry.path().extension() == ".ini") {
      loadWeaponArtFromFile(entry.path());
    }
  }
}

void Manager::Save(SKSE::SerializationInterface* a_interface)
{
  assert(a_interface);

  // Save infoMap: uint64_t -> WeaponArtInfo
  Serialization::WriteUnorderedMap<uint64_t, WeaponArtInfo>(
      a_interface, infoMap,
      [](SKSE::SerializationInterface* intfc, const uint64_t& key) {
        return Serialization::Write(intfc, key);
      },
      [](SKSE::SerializationInterface* intfc, const WeaponArtInfo& val) {
        return Serialization::Write(intfc, val.unlocked) && Serialization::Write(intfc, val.ignoreStagger) &&
               Serialization::Write(intfc, val.modifiable) && Serialization::Write(intfc, val.artType) && Serialization::Write(intfc, val.artRare) &&
               Serialization::Write(intfc, val.minPlayerLevel) && Serialization::Write(intfc, val.postureDamageMult);
      });

  // Save artMap: RE::TESObjectWEAP* -> uint64_t (persistForm)
  const std::size_t artCount = artMap.size();
  a_interface->WriteRecordData(artCount);
  for (auto& [weapPtr, persist] : artMap) {
    RE::FormID formID = weapPtr ? weapPtr->GetFormID() : 0;
    a_interface->WriteRecordData(formID);
    a_interface->WriteRecordData(persist);
  }
}

void Manager::Load(SKSE::SerializationInterface* a_interface)
{
  assert(a_interface);

  // Load infoMap
  Serialization::ReadUnorderedMap<uint64_t, WeaponArtInfo>(
      a_interface, infoMap,
      [](SKSE::SerializationInterface* intfc, uint64_t& key) {
        return Serialization::Read(intfc, key);
      },
      [](SKSE::SerializationInterface* intfc, WeaponArtInfo& val) {
        Serialization::Read(intfc, val.unlocked);
        Serialization::Read(intfc, val.ignoreStagger);
        Serialization::Read(intfc, val.modifiable);
        Serialization::Read(intfc, val.artType);
        Serialization::Read(intfc, val.artRare);
        Serialization::Read(intfc, val.minPlayerLevel);
        Serialization::Read(intfc, val.postureDamageMult);
        return true;
      });

  // Load artMap
  std::size_t artCount = 0;
  a_interface->ReadRecordData(artCount);
  artMap.clear();
  for (std::size_t i = 0; i < artCount; ++i) {
    RE::FormID oldFormID = 0;
    a_interface->ReadRecordData(oldFormID);
    bool resolved = a_interface->ResolveFormID(oldFormID, oldFormID);

    uint64_t persist = 0;
    a_interface->ReadRecordData(persist);

    if (!resolved)
      continue;

    RE::TESObjectWEAP* weap = RE::TESForm::LookupByID<RE::TESObjectWEAP>(oldFormID);
    if (weap) {
      artMap[weap] = persist;
    }
  }
}

void Manager::Revert(SKSE::SerializationInterface* a_interface)
{
  assert(a_interface);
  artMap.clear();
  infoMap.clear();
}

InfoCard::InfoCard()
{
  auto scaleformManager = RE::BSScaleformManager::GetSingleton();
  scaleformManager->LoadMovieEx(this, MENU_PATH, [](RE::GFxMovieDef* a_def) {
    a_def->SetState(RE::GFxState::StateType::kLog, RE::make_gptr<Utils::MenuLogger>().get());
    a_def->SetState(RE::GFxState::StateType::kExternalInterface, RE::make_gptr<MenuCallback>().get());
  });

  depthPriority = 0;
  inputContext  = Context::kNone;
  if (uiMovie)
    uiMovie->SetMouseCursorCount(0);

  menuFlags.set(RE::UI_MENU_FLAGS::kAlwaysOpen);
}

void InfoCard::Register()
{
  RE::UI* ui = RE::UI::GetSingleton();
  if (ui) {
    ui->Register(MENU_NAME, Creator);
  }
}

void InfoCard::Show()
{
  auto msgQ = RE::UIMessageQueue::GetSingleton();
  if (msgQ) {
    msgQ->AddMessage(InfoCard::MENU_NAME, RE::UI_MESSAGE_TYPE::kShow, nullptr);
  }
}

void InfoCard::Hide()
{
  auto msgQ = RE::UIMessageQueue::GetSingleton();
  if (msgQ) {
    msgQ->AddMessage(InfoCard::MENU_NAME, RE::UI_MESSAGE_TYPE::kHide, nullptr);
  }
}

void InfoCard::Update()
{
  auto form = Utils::GetSelectedItem();
  if (!form)
    return;
  auto weap = RE::TESForm::LookupByID(form);
  if (!weap || !weap->IsWeapon())
    return;

  auto menu = RE::UI::GetSingleton()->GetMenu(MENU_NAME);
  if (!menu || !menu->uiMovie)
    return;

  RE::GFxValue name;
  name.SetString(std::to_string(weap->GetFormID()));
  menu->uiMovie->InvokeNoReturn("updateInfo", &name, 1);
}

RE::UI_MESSAGE_RESULTS InfoCard::ProcessMessage(RE::UIMessage& a_message)
{
  if (a_message.menu != RE::InventoryMenu::MENU_NAME)
    return RE::UI_MESSAGE_RESULTS::kIgnore;
  switch (*a_message.type) {
  case RE::UI_MESSAGE_TYPE::kShow: {
    break;
  }
  case RE::UI_MESSAGE_TYPE::kHide: {
    break;
  }
  default:
    break;
  }
  return RE::UI_MESSAGE_RESULTS::kHandled;
}

void InfoCard::MenuCallback::Callback(RE::GFxMovieView* a_movieView, const char* a_methodName, const RE::GFxValue* a_args, std::uint32_t a_numArgs)
{
  static RE::GFxValue info;
  if (std::strcmp(a_methodName, "RequestInfo") == 0) {
    auto item = Utils::GetSelectedItem();
    if (!item)
      return;
    auto weap = RE::TESForm::LookupByID(item);
    if (!weap)
      return;
    if (weap->IsWeapon()) {
      WeaponArtInfo weapInfo;
      info.SetMember("name", "forTesting");
      info.SetMember("unlocked", weapInfo.unlocked);
      info.SetMember("ignoreStagger", weapInfo.ignoreStagger);
      info.SetMember("modifiable", weapInfo.modifiable);
      info.SetMember("artType", ToString(weapInfo.artType));
      info.SetMember("artRare", ToString(weapInfo.artRare));
    } else
      info.SetNull();
    a_movieView->Invoke("ReceiveInfo", nullptr, &info, 6);
  }
};
}  // namespace WeaponArt
