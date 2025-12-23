#include "setting.h"

namespace Settings
{
std::string PoiseTypeToString(PoiseType type)
{
  switch (type) {
  case PoiseType::kNull:
    return "None";
  case PoiseType::kPoise:
    return "Poise";
  case PoiseType::kChocolatePoise:
    return "ChocolatePoise";
  case PoiseType::kMaxsuPoise:
    return "MaxsuPoise";
  case PoiseType::kPoiseHandler:
    return "RimCombatPoise";
  default:
    return "Unknown";
  }
}

void InitSettings()
{
  executionMark = RE::TESDataHandler::GetSingleton()->LookupForm<RE::SpellItem>(0x0800, "ValhallaCombatRedux.esp");

  if (!executionMark) {
    logger::info("Settings: Execution Mark spell not found.");
    executionMark = nullptr;
  } else {
    logger::info("Settings: Execution Mark {}.", executionMark->GetName());
  }
}
}  // namespace Settings