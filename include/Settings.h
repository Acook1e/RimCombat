#pragma once

namespace Settings
{

#pragma region Stamina
inline bool bUseAttackStaminaSystem          = true;
inline bool bConsumeStaminaOutCombat         = true;
inline bool bNormalAttackComsumeStamina      = true;
inline float fNormalAttackStaminaCostBase    = 10.0f;
inline float fNormalAttackStaminaCostPerMass = 0.2f;
inline float fPowerAttackStaminaCostBase     = 30.0f;
inline float fPowerAttackStaminaCostPerMass  = 0.5f;

inline float fStaminaRegenMult       = 5.0f;
inline float fStaminaRegenMin        = 10.0f;
inline float fStaminaRegenLimit      = 50.0f;
inline float fStaminaRegenDelay      = 1.5f;
inline float fStaminaRegenMultCombat = 0.5f;
#pragma endregion

#pragma region Posture
inline bool bUsePostureSystem = true;
inline bool bUsePostureHUD    = true;

inline float fMaxPostureBase       = 100.0f;
inline float fMaxPostureHealthMult = 0.1f;

inline float fNormalAttackPostureDamage_Fist       = 6.0f;
inline float fNormalAttackPostureDamage_Dagger     = 8.0f;
inline float fNormalAttackPostureDamage_Sword      = 10.0f;
inline float fNormalAttackPostureDamage_Axe        = 12.0f;
inline float fNormalAttackPostureDamage_Mace       = 14.0f;
inline float fNormalAttackPostureDamage_GreatSword = 24.0f;
inline float fNormalAttackPostureDamage_GreatAxe   = 28.0f;
inline float fBashPostureDamageBase                = 32.0f;
inline float fPowerAttackPostureDamageMult         = 1.5f;
inline float fPowerBashPostureDamageMult           = 2.0f;
inline float fBlockedPostureDamageMult             = 0.5f;
inline float fBlockedPostureDamageToAttacker       = 0.25f;

inline float fArmorPostureDamageFactor = 1.1f;

inline bool bEnableExhausted             = true;
inline bool bQuitExhuastedOnHit          = true;
inline float fExhaustedRestorePercent    = 0.2f;
inline float fExhaustedPostureDamageMult = 1.5f;
#pragma endregion

template <typename T>
void SetGameSettings(const char* a_setting, T a_value);

void LoadSettings();
void SaveSettings();
}  // namespace Settings