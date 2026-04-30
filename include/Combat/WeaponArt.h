#include "Core/Serialization.h"
#include "Core/Settings.h"

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

class Manager
{
public:
  static Manager& GetSingleton()
  {
    static Manager singleton;
    return singleton;
  }

  void Init();

private:
  // RimCombat Weapon Art Info
  constexpr static inline std::uint32_t weaponInfo = 'RWAI';
  std::unordered_map<RE::TESObjectWEAP*, WeaponArtInfo> infoMap;
};
}  // namespace WeaponArt
