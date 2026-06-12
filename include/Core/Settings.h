#pragma once

namespace Settings
{
const inline std::string SettingsDir         = "Data/SKSE/Plugins/RimCombat/";
const inline std::string SettingsFile        = SettingsDir + "Settings.json";
const inline std::string SettingsDefaultFile = SettingsDir + "SettingsDefault.json";

using WeaponEnumType = std::uint8_t;
using RaceEnumType   = std::uint8_t;

#pragma region Damage
// 是否启用伤害系统
inline bool bUseDamageSystem = true;
// 重击伤害倍率，相对于轻攻击
inline float fDamageMultPowerAttack = 1.8f;
// 格挡攻击伤害倍率，相对于轻攻击
inline float fDamageMultBash = 0.50f;
// 格挡猛击伤害倍率，相对于轻攻击
inline float fDamageMultPowerBash = 0.65f;
#pragma endregion

#pragma region Stamina
// 是否启用攻击耐力系统
inline bool bUseAttackStaminaSystem = true;
// 是否在战斗外消耗耐力（如奔跑、跳跃等）
inline bool bConsumeStaminaOutCombat = true;
// 是否在耐力为零时禁用攻击
// 推荐开启，关闭后可能会影响战斗平衡
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

// 每种武器类型的基础耐力消耗
inline std::unordered_map<WeaponEnumType, float> baseStaminaCostMap{};
// 非使用武器类型的生物类型的基础耐力消耗
inline std::unordered_map<RaceEnumType, float> baseCreatureStaminaMap{};
// 每单位质量的额外耐力消耗
inline float fAttackStaminaCostPerMass = 0.2f;
// 重击耐力消耗倍率，乘以基础耐力消耗
inline float fPowerAttackStaminaCostMult = 2.5f;
// 每单位质量的重击额外耐力消耗
inline float fPowerAttackStaminaCostPerMass = 0.6f;
// 格挡攻击耐力消耗倍率，乘以基础耐力消耗
inline float fBashStaminaCostMult = 1.80f;
// 每单位质量的格挡攻击额外耐力消耗
inline float fBashStaminaCostPerMass = 0.15f;
// 格挡猛击耐力消耗倍率，乘以基础耐力消耗
inline float fPowerBashStaminaCostMult = 2.00f;
// 每单位质量的格挡猛击额外耐力消耗
inline float fPowerBashStaminaCostPerMass = 0.20f;
#pragma endregion

#pragma region Posture
// 是否启用架势系统
inline bool bUsePostureSystem = true;
// 是否使用TrueHUD显示架势值
inline bool bUsePostureHUD = true;
// 是否禁用玩家失衡
inline bool bDisablePlayerPostureBreak = false;

// 每点生命值对应的最大架势值增加量
inline float fMaxPostureHealthMult = 0.06f;
// 每点耐力值对应的最大架势值增加量
inline float fMaxPostureStaminaMult = 0.04f;
// 架势值恢复的延迟，单位为毫秒
inline std::uint64_t uPostureRegenDelay = 5000;
// 每秒恢复的架势值百分比
inline float fPostureRegenPercentPerSecond = 3.0f;

// 各个种族的基础架势值
inline std::unordered_map<RaceEnumType, float> basePostureMap{};

// 非使用武器类型的生物基础架势伤害
inline std::unordered_map<RaceEnumType, float> baseCreaturePostureDamage{};

// 每种武器类型的基础架势伤害
inline std::unordered_map<WeaponEnumType, float> basePostureDamageMap{};

// 格挡攻击架势伤害倍率，乘以基础架势伤害
inline float fBashPostureDamageMult = 0.50f;
// 重击架势伤害倍率，乘以基础架势伤害
inline float fPowerAttackPostureDamageMult = 2.00f;
// 格挡猛击架势伤害倍率，乘以基础架势伤害
inline float fPowerBashPostureDamageMult = 0.65f;

// 护甲值减少的架势伤害计算因子
inline float fArmorPostureDamageFactor = 1.1f;
#pragma endregion

#pragma region Poise
// 是否启用韧性系统
inline bool bUsePoiseSystem = true;
// 每点耐力值对应的韧性增加量
inline float fPoiseStaminaMult = 0.02f;
// 每单位质量对应的韧性增加量
inline float fPoiseMassMult = 1.2f;
// 韧性恢复的延迟，单位为毫秒
inline std::uint64_t uPoiseRegenDelay = 3000;
// 每秒恢复的韧性值百分比
inline float fPoiseRegenPercentPerSecond = 15.0f;
// 硬直补偿的最低最大韧性百分比伤害
inline float fStaggerCompensationPercent = 0.70f;
// 造成小型、中型、大型硬直的冲击力阈值
inline float fStaggerLevelSmall  = 2.0f;
inline float fStaggerLevelMedium = 5.0f;
inline float fStaggerLevelLarge  = 9.0f;
// 轻甲对应部位的最大韧性值修正
inline float fLightArmorHeadMaxPoiseBonus = 0.35f;
inline float fLightArmorBodyMaxPoiseBonus = 1.0f;
inline float fLightArmorHandMaxPoiseBonus = 0.2f;
inline float fLightArmorFeetMaxPoiseBonus = 0.2f;
// 重甲对应部位的最大韧性值修正
inline float fHeavyArmorHeadMaxPoiseBonus = 0.8f;
inline float fHeavyArmorBodyMaxPoiseBonus = 2.5f;
inline float fHeavyArmorHandMaxPoiseBonus = 0.6f;
inline float fHeavyArmorFeetMaxPoiseBonus = 0.6f;
// 格挡攻击韧性伤害倍率，乘以基础韧性伤害
inline float fBashPoiseDamageMult = 0.50f;
// 重击的韧性伤害倍率，乘以基础韧性伤害
inline float fPowerAttackPoiseDamageMult = 2.00f;
// 格挡猛击韧性伤害倍率，乘以基础韧性伤害
inline float fPowerBashPoiseDamageMult = 0.65f;
// 等级差距存在时，攻击者的韧性伤害倍率衰减的因子
inline float fLevelDiffAggressorMultPerLevel = 0.05f;
// 攻击过程中，基于的额外韧性值，相对于受击者此次攻击削韧值的百分比
inline float fVictimAttackingPoiseBonusPercent = 0.8f;

// 各个种族的基础韧性值
inline std::unordered_map<RaceEnumType, float> basePoiseMap{};

// 每种武器类型的基础韧性伤害
inline std::unordered_map<WeaponEnumType, float> basePoiseDamageMap{};

// 非使用武器类型的生物基础韧性伤害
inline std::unordered_map<RaceEnumType, float> baseCreaturePoiseDamage{};

#pragma endregion

#pragma region Stagger
// 是否启用硬直系统
inline bool bUseStaggerSystem = true;

// 受到小型，中型，大型硬直的恢复时间，单位为毫秒
inline std::uint64_t uStaggerRecoveryTimeSmall  = 500;
inline std::uint64_t uStaggerRecoveryTimeMedium = 900;
inline std::uint64_t uStaggerRecoveryTimeLarge  = 1300;

#pragma endregion

#pragma region Exhausted
// 是否启用力竭系统
inline bool bUseExhaustedSystem = true;
// 力竭状态是否禁用玩家攻击
// 默认关闭，按需开启，几乎不影响战斗平衡，但能增加一些沉浸感
inline bool bDisablePlayerAttackWhenExhausted = false;
// 力竭状态是否禁用NPC攻击
// 推荐开启，否则NPC会无限制攻击玩家，导致玩家无法反击
inline bool bDisableNPCAttackWhenExhausted = true;
// 力竭受击时是否退出力竭状态
inline bool bExitExhaustedOnHit = true;
// 退出力竭状态的耐力值百分比
inline float fExhaustedExitPercent = 0.2f;
// 力竭状态下的攻击伤害倍率，乘以原始伤害
inline float fAttackDamageMultWhenExhausted = 0.7f;
// 力竭状态下的攻击架势伤害倍率，乘以基础架势伤害
inline float fAttackPostureDamageMultWhenExhausted = 0.7f;
// 力竭状态的受击倍率
inline float fOnHitDamageMultWhenExhausted = 1.5f;
// 力竭状态下的收到架势伤害倍率，乘以基础架势伤害
inline float fOnHitPostureDamageMultWhenExhausted = 1.5f;
#pragma endregion

#pragma region Block
// 是否启用格挡系统
inline bool bUseBlockSystem = true;
// 是否启用限时格挡系统
inline bool bTimedBlockEnabled = true;
// 是否允许限时格挡完全免疫破防
inline bool bTimedBlockNeverPostureBreak = true;
// 限时格挡的时间窗口，单位为毫秒
inline std::uint64_t uTimedBlockLimit = 150;
// 限时格挡触发持续一段时间的效果，单位为毫秒
// 在这段时间内，触发者免疫后座力/硬直，并且被认为是限时格挡
inline std::uint64_t uTimedBlockDuration = 500;
// 一次格挡时最多消耗的耐力百分比
inline float fBlockMaxStaminaConsumePercent = 0.5f;
// 耐力不足消耗的时候，附带的额外耐力数值
inline float fBlockMinStaminaConsume = 10.0f;

// 每种武器类型的格挡强度，影响格挡时的伤害减免和架势伤害倍率
inline std::unordered_map<WeaponEnumType, float> blockStrengthMap{};
// 限时格挡的额外格挡强度乘数
inline float fTimedBlockBlockStrengthMult = 4.0f;
#pragma endregion

#pragma region WeaponArt
// 是否启用战技系统
inline bool bUseWeaponArtSystem = true;
// 是否启用战技HUD
inline bool bUseWeaponArtHUD = true;
// 是否在收刀的时候隐藏HUD
inline bool bHideWeaponArtHUDOnSheathe = true;
// 战技HUD横向位置，单位为屏幕百分比
inline float fWeaponArtHUDPosX = 6.0f;
// 战技HUD纵向位置，单位为屏幕百分比
inline float fWeaponArtHUDPosY = 70.0f;
// 战技HUD整体缩放
inline float fWeaponArtHUDScale = 1.0f;
// 战技菜单起始横向位置，单位为屏幕百分比
inline float fWeaponArtMenuStartPercent = 50.0f;
#pragma endregion

#pragma region Execution
// 是否启用处决系统
inline bool bUseExecutionSystem = true;
// 处决状态受击是受到的伤害倍率，乘以原始伤害
inline float fOnHitDamageMultWhenExecutable = 1.8f;

// 每种武器类型的处决伤害倍率
inline std::unordered_map<WeaponEnumType, float> executionDamageMultMap{};
#pragma endregion

void UpdateGameSettings();
void LoadSettings();
void ResetSettings();
void SaveSettings();
}  // namespace Settings