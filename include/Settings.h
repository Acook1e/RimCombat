#pragma once

namespace Settings
{
inline std::string SettingsDir;

#pragma region Stamina
inline bool bUseAttackStaminaSystem     = true;
inline bool bConsumeStaminaOutCombat    = true;
inline bool bNormalAttackComsumeStamina = true;
inline bool bDisableAttackStaminaZero   = true;

inline float fStaminaRegenMult       = 5.0f;
inline float fStaminaRegenMin        = 10.0f;
inline float fStaminaRegenMax        = 50.0f;
inline float fStaminaRegenDelay      = 1.5f;
inline float fStaminaRegenMultCombat = 0.5f;
inline float fStaminaRegenMultBlock  = 0.5f;

inline float fNormalAttackStaminaCostBase_Fist       = 5.0f;
inline float fNormalAttackStaminaCostBase_Dagger     = 6.0f;
inline float fNormalAttackStaminaCostBase_Sword      = 8.0f;
inline float fNormalAttackStaminaCostBase_Axe        = 8.0f;
inline float fNormalAttackStaminaCostBase_Mace       = 8.0f;
inline float fNormalAttackStaminaCostBase_GreatSword = 15.0f;
inline float fNormalAttackStaminaCostBase_GreatAxe   = 15.0f;
inline float fNormalAttackStaminaCostPerMass         = 0.2f;
inline float fPowerAttackStaminaCostMult             = 2.5f;
inline float fPowerAttackStaminaCostPerMass          = 0.5f;
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
inline float fBlockPostureDamageMult               = 0.5f;
inline float fBlockPostureDamageToAttacker         = 0.1f;

inline float fArmorPostureDamageFactor = 1.1f;

inline bool bEnableExhausted             = true;
inline bool bExitExhaustedOnHit          = true;
inline float fExhaustedRestorePercent    = 0.2f;
inline float fExhaustedPostureDamageMult = 1.5f;
#pragma endregion

#pragma region Block
inline bool bTimedBlockEnabled           = true;
inline bool bTimedBlockNeverPostureBreak = true;
inline int32_t iTimedBlockLimit          = 150;  // ms

inline float fBlockDamageMult                   = 0.80f;
inline float fTimedBlockDamageMult              = 0.95f;
inline float fBlockStaminaConsumePerDamage      = 0.5f;
inline float fTimedBlockStaminaConsumePerDamage = 0.2f;
inline float fTimedBlockPostureDamageMult       = 0.2f;
#pragma endregion

void UpdateGameSettings();
void LoadSettings();
void SaveSettings();

// 1-32 = suffix
// 33-64 = hash
void AddHashMapping(uint32_t hash, uint32_t prefix);
uint64_t toPersistForm(uint32_t hash, uint32_t suffix);
RE::TESForm* toTESForm(uint64_t form);
}  // namespace Settings