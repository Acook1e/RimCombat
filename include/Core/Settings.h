#pragma once

namespace Settings
{
constexpr inline std::string_view SettingsFile = "Data/SKSE/Plugins/RimCombat/Settings.json";
constexpr inline std::string_view SettingsDir  = "Data/SKSE/Plugins/RimCombat/";

#pragma region Stamina
// 是否启用攻击耐力系统
inline bool bUseAttackStaminaSystem = true;
// 是否在战斗外消耗耐力（如奔跑、跳跃等）
inline bool bConsumeStaminaOutCombat = true;
// 是否普通攻击消耗耐力
inline bool bNormalAttackComsumeStamina = true;
// 是否在玩家耐力为零时禁用攻击
// 推荐开启，关闭后可能会影响战斗平衡
// 另外不设置耐力为零禁用NPC攻击的选项
// 推荐使用力竭系统来实现NPC攻击禁用，更加灵活且具有沉浸感
inline bool bDisablePlayerAttackWhenStaminaZero = true;

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
// 架势值恢复的延迟，单位为毫秒
inline std::uint64_t uPostureRegenDelay = 2000;
// 每秒恢复的架势值百分比
inline float fPostureRegenPercentPerSecond = 5.0f;

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

// 护甲值减少的架势伤害计算因子
inline float fArmorPostureDamageFactor = 1.1f;
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

// 格挡强度，通过这个数值计算
// 格挡时每点伤害对应的耐力消耗
// 格挡时的最大伤害减免
// 格挡时的架势伤害乘数
// 格挡时返还的架势伤害乘数

// 空手格挡强度
inline float fBlockStrength_Unarm = 8.0f;
// 匕首格挡强度
inline float fBlockStrength_Dagger = 16.0f;
// 单手剑格挡强度
inline float fBlockStrength_Sword = 22.0f;
// 单手斧格挡强度
inline float fBlockStrength_Axe = 24.0f;
// 单手锤格挡强度
inline float fBlockStrength_Mace = 26.0f;
// 双手剑格挡强度
inline float fBlockStrength_GreatSword = 36.0f;
// 双手斧格挡强度
inline float fBlockStrength_GreatAxe = 38.0f;
// 双手锤格挡强度
inline float fBlockStrength_GreatMace = 42.0f;
// 盾牌格挡强度
inline float fBlockStrength_Shield = 48.0f;
// 拳套格挡强度
inline float fBlockStrength_Fist = 10.0f;
// 限时格挡的额外格挡强度乘数
inline float fTimedBlockBlockStrengthMult = 6.0f;
#pragma endregion

#pragma region WeaponArt
// 是否启用战技系统
inline bool bUseWeaponArtSystem = true;
// 是否启用战技HUD
inline bool bUseWeaponArtHUD = true;
// 战技HUD横向位置，单位为屏幕百分比
inline float fWeaponArtHUDPosX = 10.0f;
// 战技HUD纵向位置，单位为屏幕百分比
inline float fWeaponArtHUDPosY = 70.0f;
// 战技HUD整体缩放
inline float fWeaponArtHUDScale = 1.0f;
#pragma endregion

#pragma region Execution
// 是否启用处决系统
inline bool bUseExecutionSystem = true;
// 是否启用受击退出处决状态
inline bool bExitExecutionOnHit = true;
// 处决状态的持续时间，单位为毫秒
inline std::uint64_t uExecutableDuration = 5000;
// 处决状态受击是受到的伤害倍率，乘以原始伤害
inline float fOnHitDamageMultWhenExecutable = 1.8f;

// 处决伤害倍率，由右手类型决定倍率
// 由右手基础伤害作为基础伤害
// 设定要高于重击，且为真实伤害，因此至少为2.0倍

// 空手处决伤害倍率
inline float fExecutionDamageMult_Unarm = 3.0f;
// 匕首处决伤害倍率
inline float fExecutionDamageMult_Dagger = 8.0f;
// 单手剑处决伤害倍率
inline float fExecutionDamageMult_Sword = 3.0f;
// 单手斧处决伤害倍率
inline float fExecutionDamageMult_Axe = 3.2f;
// 单手锤处决伤害倍率
inline float fExecutionDamageMult_Mace = 3.5f;
// 双手剑处决伤害倍率
inline float fExecutionDamageMult_GreatSword = 2.0f;
// 双手斧处决伤害倍率
inline float fExecutionDamageMult_GreatAxe = 2.2f;
// 双手锤处决伤害倍率
inline float fExecutionDamageMult_GreatMace = 2.3f;
// 拳套处决伤害倍率
inline float fExecutionDamageMult_Fist = 3.0f;
#pragma endregion

void UpdateGameSettings();
void LoadSettings();
void SaveSettings();
}  // namespace Settings