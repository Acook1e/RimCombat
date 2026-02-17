#include "Serialization.h"
#include "Settings.h"

namespace WeaponArt
{
enum WeaponArtType : uint8_t
{
  kOnce,
  kStage,
  kStance
};
std::string_view ToString(WeaponArtType type);

enum WeaponArtRare : uint8_t
{
  kCommon,
  kRare,
  kEpic,
  kLegendary
};
std::string_view ToString(WeaponArtRare rare);

struct WeaponArtInfo
{
  bool unlocked;
  bool ignoreStagger;
  bool modifiable;
  WeaponArtType artType;
  WeaponArtRare artRare;
  uint8_t minPlayerLevel;
  uint8_t postureDamageMult;
};

class Manager : public Serialization
{
public:
  static Manager& GetSingleton()
  {
    static Manager singleton;
    return singleton;
  }

  void Init();
  void Save(SKSE::SerializationInterface* a_interface);
  void Load(SKSE::SerializationInterface* a_interface);
  void Revert(SKSE::SerializationInterface* a_interface);

private:
  // RE::TESObjectWEAP* -> uint64_t (RE::SpellItem*)
  std::unordered_map<RE::TESObjectWEAP*, uint64_t> artMap;
  // uint64_t (RE::SpellItem*) -> WeaponArtInfo
  std::unordered_map<uint64_t, WeaponArtInfo> infoMap;
};

class InfoCard : public RE::IMenu
{
public:
  static constexpr std::string_view MENU_PATH = "WeaponArtInfoCard.swf";
  static constexpr std::string_view MENU_NAME = MENU_PATH.substr(0, MENU_PATH.size() - 4);

  InfoCard();
  ~InfoCard() override = default;

  static void Register();
  static void Show();
  static void Hide();
  static void Update();

  RE::UI_MESSAGE_RESULTS ProcessMessage(RE::UIMessage& a_message) override;

  static RE::stl::owner<RE::IMenu*> Creator() { return new InfoCard(); }

private:
  class MenuCallback : public RE::GFxExternalInterface
  {
    void Callback(RE::GFxMovieView* a_movieView, const char* a_methodName, const RE::GFxValue* a_args, std::uint32_t a_numArgs) override;
  };
};
}  // namespace WeaponArt
