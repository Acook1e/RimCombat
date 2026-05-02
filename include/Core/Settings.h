#pragma once

namespace Settings
{
constexpr inline std::string_view SettingsFile =
    "Data/SKSE/Plugins/RimCombat/Settings.json";
constexpr inline std::string_view SettingsDir = "Data/SKSE/Plugins/RimCombat/";

#pragma region Stamina
// 是否启用攻击耐力系统
inline bool bUseAttackStaminaSystem = true;
// 是否在战斗外消耗耐力（如奔跑、跳跃等）
inline bool bConsumeStaminaOutCombat = true;
// 是否普通攻击消耗耐力
inline bool bNormalAttackComsumeStamina = true;
// 是否在耐力为零时禁用攻击
inline bool bDisableAttackWhenStaminaZero = true;

// 全局耐力再生倍率
inline float fStaminaRegenMult = 5.0f;
// 全局耐力再生最小值
inline float fStaminaRegenMin = 10.0f;
// 全局耐力再生最大值
inline float fStaminaRegenMax = 50.0f;
// 耐力再生延迟，单位为秒
inline float fStaminaRegenDelay = 1.5f;
// 战斗中耐力再生倍率
inline float fStaminaRegenMultCombat = 1.0f;
// 格挡时耐力再生倍率
inline float fStaminaRegenMultBlock = 0.5f;

// 空手基础耐力消耗
inline float fNormalAttackStaminaCostBase_Unarm = 5.0f;
// 匕首基础耐力消耗
inline float fNormalAttackStaminaCostBase_Dagger = 6.0f;
// 单手剑基础耐力消耗
inline float fNormalAttackStaminaCostBase_Sword = 8.0f;
// 单手斧基础耐力消耗
inline float fNormalAttackStaminaCostBase_Axe = 8.0f;
// 单手锤基础耐力消耗
inline float fNormalAttackStaminaCostBase_Mace = 8.0f;
// 双手剑基础耐力消耗
inline float fNormalAttackStaminaCostBase_GreatSword = 15.0f;
// 双手斧基础耐力消耗
inline float fNormalAttackStaminaCostBase_GreatAxe = 15.0f;
// 双手锤基础耐力消耗
inline float fNormalAttackStaminaCostBase_GreatMace = 15.0f;
// 拳套基础耐力消耗
inline float fNormalAttackStaminaCostBase_Fist = 5.0f;
// 每单位质量的额外耐力消耗，适用于所有武器类型
inline float fNormalAttackStaminaCostPerMass = 0.2f;
// 重击耐力消耗倍率，乘以基础耐力消耗
inline float fPowerAttackStaminaCostMult = 2.5f;
// 每单位质量的重击额外耐力消耗，适用于所有武器类型
inline float fPowerAttackStaminaCostPerMass = 0.5f;
#pragma endregion

#pragma region Posture
// 是否启用架势系统
inline bool bUsePostureSystem = true;
// 是否使用TrueHUD显示架势值
inline bool bUsePostureHUD = true;

// 默认最大架势值
inline float fMaxPostureBase = 100.0f;
// 每点生命值对应的最大架势值增加量
inline float fMaxPostureHealthMult = 0.1f;

// 空手基础架势伤害
inline float fNormalAttackPostureDamage_Unarm = 6.0f;
// 匕首基础架势伤害
inline float fNormalAttackPostureDamage_Dagger = 8.0f;
// 单手剑基础架势伤害
inline float fNormalAttackPostureDamage_Sword = 10.0f;
// 单手斧基础架势伤害
inline float fNormalAttackPostureDamage_Axe = 12.0f;
// 单手锤基础架势伤害
inline float fNormalAttackPostureDamage_Mace = 14.0f;
// 双手剑基础架势伤害
inline float fNormalAttackPostureDamage_GreatSword = 24.0f;
// 双手斧基础架势伤害
inline float fNormalAttackPostureDamage_GreatAxe = 28.0f;
// 双手锤基础架势伤害
inline float fNormalAttackPostureDamage_GreatMace = 32.0f;
// 盾牌基础架势伤害
inline float fBashPostureDamage_Shield = 32.0f;
// 拳套基础架势伤害
inline float fNormalAttackPostureDamage_Fist = 6.0f;

// 格挡攻击架势伤害倍率，乘以基础架势伤害
inline float fBashPostureDamageMult = 2.0f;
// 重击架势伤害倍率，乘以基础架势伤害
inline float fPowerAttackPostureDamageMult = 1.5f;
// 格挡猛击架势伤害倍率，乘以基础架势伤害
inline float fPowerBashPostureDamageMult = 2.5f;
// 格挡时受击者的架势伤害乘数，乘以基础架势伤害
inline float fBlockPostureDamageMult = 0.5f;
// 格挡时返回给攻击者的架势伤害乘数
inline float fBlockPostureDamageToAttacker = 0.1f;

// 护甲值减少的架势伤害计算因子
inline float fArmorPostureDamageFactor = 1.1f;
#pragma endregion

#pragma region Exhausted
// 是否启用力竭系统
inline bool bEnableExhausted = true;
// 力竭受击时是否退出力竭状态
inline bool bExitExhaustedOnHit = true;
// 退出力竭状态的耐力值百分比
inline float fExhaustedExitPercent = 0.2f;
// 力竭状态的受击倍率
inline float fExhaustedHitDamageMult = 1.8f;
// 力竭状态下的架势伤害倍率，乘以基础架势伤害
inline float fExhaustedPostureDamageMult = 1.5f;
#pragma endregion

#pragma region Block
// 是否启用限时格挡系统
inline bool bTimedBlockEnabled = true;
// 是否允许限时格挡完全免疫破防
inline bool bTimedBlockNeverPostureBreak = true;
// 限时格挡的时间窗口，单位为毫秒
inline int32_t iTimedBlockLimit = 150;  // ms

// 格挡时减免伤害的倍率，乘以原始伤害
inline float fBlockDamageMult = 0.80f;
// 限时格挡时减免伤害的倍率，乘以原始伤害
inline float fTimedBlockDamageMult = 0.95f;
// 格挡时每点伤害对应的耐力消耗
inline float fBlockStaminaConsumePerDamage = 0.5f;
// 限时格挡时每点伤害对应的耐力消耗
inline float fTimedBlockStaminaConsumePerDamage = 0.2f;
// 限时格挡受到的架势伤害倍率，乘以基础架势伤害
inline float fTimedBlockPostureDamageMult = 0.2f;
#pragma endregion

void UpdateGameSettings();
void LoadSettings();
void SaveSettings();
}  // namespace Settings