#pragma once

#include "pch.h"

namespace Settings
{
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

inline bool bUseExhaustionSystem   = true;
inline float fExhaustionEndPercent = 0.6;
inline float fExhaustionDamageMult = 0.5;

inline float fStaminaRegenMult         = 5.0f;
inline float fStaminaRegenLimit        = 50.0f;
inline float fStaminaRegenMin          = 10.0f;
inline float fCombatStaminaRegenMult   = 1.0f;
inline float fBlockingStaminaRegenMult = 0.5f;
inline float fStaminaRegenDelay        = 1.0f;

inline bool bBlockStaminaToggle = true;

#pragma endregion
#pragma region Poise&Execution

inline bool bEnablePoiseExecution = true;
inline bool bUsePoiseHUD          = true;
inline bool bHudEnabled           = false;
inline float fPoiseBreakStunTime  = 5.0f;

inline bool bRestorePoiseInCombat         = true;
inline float fRestorePoiseSpeed           = 0.001f;
inline float fRestorePoiseSpeedCombatMult = 0.05f;

inline float fNormalAttackPoiseDamage_Fist       = 5.0f;
inline float fNormalAttackPoiseDamage_Dagger     = 6.0f;
inline float fNormalAttackPoiseDamage_Sword      = 8.0f;
inline float fNormalAttackPoiseDamage_Axe        = 10.0f;
inline float fNormalAttackPoiseDamage_Mace       = 12.0f;
inline float fNormalAttackPoiseDamage_GreatSword = 20.0f;
inline float fNormalAttackPoiseDamage_GreatAxe   = 22.0f;
inline float fBashPoiseDamageBase                = 16.0f;
inline float fActorMaxPoiseBase                  = 100.0f;

inline float fPowerAttackPoiseDamageMult   = 200.0f;
inline float fBlockedPoiseDamageMult       = 0.5f;
inline float fBlockedPoiseDamageToAttacker = 0.0f;
inline float fPowerBashPoiseDamageMult     = 1.5f;

inline bool bEnablePlayerAutoExecution = true;
inline bool bEnableNPCAutoExecution    = true;
inline float fExecutionMaxDistance     = 200.0f;

inline float fExecutionHealthPercent = 0.06f;

#pragma endregion

template <typename T>
void SetGameSettings(const char* a_setting, T a_value);

void InitSettings();
void LoadSettings();
void SaveSettings();
}  // namespace Settings