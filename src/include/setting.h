#pragma once

namespace Settings
{
enum PoiseType : uint8_t
{
  kNull = 0,
  kPoise,
  kChocolatePoise,
  kMaxsuPoise,
  kPoiseHandler,
  kTotal
};
std::string PoiseTypeToString(PoiseType type);

#pragma region Stamina

inline bool bConsumeStaminaOutCombat = true;

inline bool bUseAttackStaminaSystem = true;

inline bool bNormalAttackComsumeStamina      = true;
inline bool bNormalAttackComsumeStaminaCheck = true;
inline float fNormalAttackStaminaCostBase    = 5;
inline float fNormalAttackStaminaCostPerMass = 0.2;

inline bool bPowerAttackComsumeStaminaTweak = true;
inline bool bPowerAttackComsumeStaminaCheck = true;
inline float fPowerAttackStaminaCostBase    = 30;
inline float fPowerAttackStaminaCostPerMass = 0.5;

inline bool bUseExhaustionSystem     = true;
inline float fExhaustionEndPercent   = 0.6;
inline float fExhaustionDamageMult   = 0.5;
inline RE::SpellItem* exhaustionMark = nullptr;

inline float fStaminaRegenMult         = 5;
inline float fStaminaRegenLimit        = 50;
inline float fStaminaRegenMin          = 10;
inline float fCombatStaminaRegenMult   = 1;
inline float fBlockingStaminaRegenMult = 0.5;
inline float fStaminaRegenDelay        = 2;

inline bool bBlockStaminaToggle = true;

#pragma endregion
#pragma region Poise

inline bool bUsePoiseHUD         = true;
inline bool bHudEnabled          = false;
inline PoiseType poiseType       = PoiseType::kPoiseHandler;
inline float fPoiseBreakStunTime = 5.0f;

inline bool bRestorePoiseInCombat = true;

inline float fNormalAttackPoiseDamage_Fist       = 5.0f;
inline float fNormalAttackPoiseDamage_Dagger     = 6.0f;
inline float fNormalAttackPoiseDamage_Sword      = 8.0f;
inline float fNormalAttackPoiseDamage_Axe        = 10.0f;
inline float fNormalAttackPoiseDamage_Mace       = 12.0f;
inline float fNormalAttackPoiseDamage_GreatSword = 20.0f;
inline float fNormalAttackPoiseDamage_GreatAxe   = 22.0f;
inline float fBashPoiseDamage                    = 16.0f;

inline float fPowerAttackPoiseDamageMult   = 2.0f;
inline float fBlockedPoiseDamageMult       = 0.5f;
inline float fBlockedPoiseDamageToAttacker = 200.0f;
inline float fPowerBashPoiseDamageMult     = 1.5f;

#pragma endregion
#pragma region Execution

inline bool bEnableExecution        = true;
inline RE::SpellItem* executionMark = nullptr;
inline RE::TESIdleForm* victimIdle  = nullptr;
inline std::unordered_map<int, RE::TESIdleForm*> attackerIdles;

inline float fExecutionHealthPercent = 0.06f;

#pragma endregion

void InitSettings();
void LoadSettings();
void SaveSettings();
}  // namespace Settings
