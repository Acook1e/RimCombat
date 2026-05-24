# Rim Combat Wiki

> 这份 WIKI 以当前源码为准，不以旧版设计稿或旧 README 为准。
>
> 下文默认不把处决系统计入当前完成度。处决代码仍保留在项目里，但由于 OAR 侧联动问题，当前处于暂缓状态。

---

## 1. 模组概览

Rim Combat 是一套面向 Skyrim SE/AE 的近战战斗重构原型。它现在的重点不是单纯叠动作，而是把近战中的“资源承诺、压制、博弈和扩展接口”先搭成一条稳定可测的主循环。

当前可玩的核心链路主要包括：

- 武器分类与按类型取值
- 攻击耐力与力竭
- 架势与破防
- 内部韧性与硬直
- 格挡与限时格挡
- 战技分配、切换、施放与 HUD / 菜单支持

从当前代码状态来看，这套系统已经可以进游戏做持续测试和数值迭代，但仍然更接近“可玩原型”，而不是“已经收尾的正式战斗 overhaul”。

---

## 2. 开启前后有什么变化

| 范围 | 未启用或接近原版时 | 启用 Rim Combat 后 |
| --- | --- | --- |
| 攻击资源 | 攻击大体沿用原版耐力逻辑 | 耐力消耗由武器类型、武器重量、攻击类型、Precision 事件和动画 payload 共同决定 |
| 耐力见底 | 更多像软惩罚 | 可以直接禁止攻击，并进一步进入力竭状态 |
| 防御 | 格挡更偏向单纯减伤 | 格挡强度、耐力换算、架势影响、限时格挡窗口与持续收益都会参与 |
| 破防节奏 | 没有独立架势层 | 架势成为独立资源，带有种族基础值、恢复延迟和破防处理 |
| 受击反馈 | 主要依赖原版 stagger | 变成“内部韧性阈值 + 硬直等级 + 恢复时间 + 特殊硬直扩展”的组合 |
| 特殊能力 | 以固定技能/法术为主 | 战技可以按武器分配、切换启用、进入准备态，并接法术派生 |
| 调参与 UI | 更分散、更静态 | 统一的 JSON 设置、本地化文本、HUD 状态显示和背包侧右半屏分配菜单 |

---

## 3. 当前各系统说明

### 武器分类与基础数值层

- 插件会在原版武器分类基础上进一步细分武器类型。
- 安装 OCF 后，可以识别更细的子类型，例如细剑、武士刀、枪矛、长柄、鞭、长棍等。
- 基础耐力消耗、架势伤害、韧性伤害、格挡强度、处决倍率等数值，都是按武器类型读取。
- 如果没有 OCF，系统仍然能工作，但会退回更粗的分类路径。

### 攻击耐力与力竭

- 原版 swing 事件、Precision 碰撞事件，以及 Rim Combat 自己的 payload 事件，都会汇入同一套耐力系统。
- 普攻、重击、武器重量修正、耐力再生倍率、战斗内再生、格挡时再生都可独立配置。
- 力竭状态可以禁止攻击、削弱输出、放大受击惩罚，并支持被击中后自动退出。
- 玩家与 NPC 是否会在力竭时被禁止攻击，可以分开配置。

### 格挡与限时格挡

- 格挡开始时间与限时格挡窗口都有明确记录。
- 限时格挡可以提供一段持续时间内的受益，而不仅仅是“瞬间判断一次”。
- 它会影响减伤、耐力消耗、架势处理以及对后座/硬直的抵抗。
- 就当前版本而言，格挡是完成度最高的切片之一。

### 架势

- 最大架势由生命、耐力和种族基础值共同决定。
- 架势伤害由武器类型和攻击类型决定。
- Bash、Power Attack、Power Bash 各自有独立倍率。
- 护甲会通过独立因子减少承受的架势伤害。
- 架势 HUD 当前走的是 TrueHUD 联动路径。

### 韧性与硬直

- 韧性是内部资源，用来决定这次受击最终会落到哪个硬直等级。
- 最大韧性由耐力、装备质量、护甲部位加成和种族基础值共同决定。
- 韧性伤害以武器类型为基础，并支持通过动画 payload 分别修正“自身受击”和“打到目标”两类倍率。
- 硬直系统支持恢复时间配置，并扩展了额外等级，例如击倒、挑飞、击飞。
- 现在命中硬直等级会优先保留更高等级，不会再被较弱的 payload 直接覆盖掉。
- 同等级硬直再次命中时，会刷新恢复时间，但不会强制重复播放同一级硬直动画。
- 远程/法术类的特殊硬直可以通过 `Stagger` 目录下的 JSON 做映射。

### 战技系统

- 战技从 JSON 文件读取，并把对象键哈希成运行时 ID。
- 每把武器都可以保存自己的战技分配结果。
- 还支持按种族路由的特殊空手形态战技 ID，例如普通空手、狼人、熊人、吸血鬼领主。
- 战技可以设置解锁等级、点数消耗、是否需要准备态，以及法术派生数据。
- 玩家目前会根据战技造成的伤害获取经验，并在升级时获得战技点数。
- 现阶段玩家侧和特殊生物形态接口已经足够支持测试，但 NPC 使用和内容量仍然明显不足。

### UI、本地化与设置

- PrismaUI 负责战技菜单与 HUD。
- 战技分配菜单会在背包打开时作为右半屏伴随面板出现，并跟随当前选中的武器刷新。
- HUD 的状态与战技名称由 C++ 侧直接推送。
- 设置按分组写入 JSON，并支持通过 `SettingsDefault.json` 覆盖重置。
- 本地化文本单独存放，再由原生侧解析后发往 UI。

### 处决当前状态

- 处决相关 hook、状态和 payload 路由还在代码里。
- 但它当前不计入整体完成度，也不建议当成当前版本的完整玩法来使用。
- 现阶段文档只把它当作保留中的暂缓模块。

---

## 4. 当前操作、文件与兼容说明

### 当前操作方式

- 背包菜单打开时，按 `T` 开关战技分配菜单。
- 按 `Left Alt` 切换玩家的战技状态。

### 运行时文件

- `Data/SKSE/Plugins/RimCombat/Settings.json`：当前实际生效的设置文件
- `Data/SKSE/Plugins/RimCombat/SettingsDefault.json`：重置设置时的默认来源
- `Data/SKSE/Plugins/RimCombat/WeaponArt/*.json`：战技定义文件
- `Data/SKSE/Plugins/RimCombat/Stagger/*.json`：远程/法术硬直映射文件
- `Data/SKSE/Plugins/RimCombat/Localization.json`：本地化与 UI 文本

### 推荐搭配

- 必需：SKSE
- 动画链路推荐：OAR、MCO 或 BFCO
- UI 推荐：PrismaUI
- 架势反馈推荐：TrueHUD
- 硬直生态推荐：Maxsu Poise、Modern Stagger Lock
- 可选：OCF，用于更细的武器分类

---

## 5. 设置项说明

### 总说明

- `ResetSettings()` 会先用 `SettingsDefault.json` 覆写 `Settings.json`，然后重新加载并重新应用设置。
- 当前代码真正会读取的键只有下面列出的这些。
- 按武器类型取值的 map，会使用武器枚举名，例如 `Dagger`、`Sword`、`Rapier`、`Katana`、`GreatSword`、`Pike`、`Bow`、`Shield`、`Torch`。
- 按种族类型取值的 map，会使用默认文件里已经生成好的种族枚举名。实际使用时，最稳妥的方式就是直接改现有默认文件里的条目。
- `Poise` 分组读取时仍兼容旧键名 `ImpactLevelSmall`、`ImpactLevelMedium`、`ImpactLevelLarge`，但保存时会统一写成新的 `StaggerLevel*`。
- 如果你看到样例设置里还有这里没有列出的旧版处决键名，那些属于遗留字段，当前代码不会读取。

### Stamina

| 键名 | 作用 |
| --- | --- |
| `UseAttackStaminaSystem` | 自定义攻击耐力系统总开关 |
| `ConsumeStaminaOutCombat` | 是否在非战斗场景也沿用这套耐力处理 |
| `DisableAttackWhenStaminaZero` | 耐力为 0 时是否禁止攻击 |
| `StaminaRegenMult` | 全局耐力再生倍率 |
| `StaminaRegenMin` | 耐力再生下限 |
| `StaminaRegenMax` | 耐力再生上限 |
| `StaminaRegenDelay` | 耐力再生恢复前的延迟 |
| `StaminaRegenMultCombat` | 战斗中耐力再生倍率 |
| `StaminaRegenMultBlock` | 格挡时耐力再生倍率 |
| `NormalAttackStaminaCostPerMass` | 普攻每单位武器质量附加的耐力消耗 |
| `PowerAttackStaminaCostMult` | 重击相对基础耐力消耗的倍率 |
| `PowerAttackStaminaCostPerMass` | 重击每单位武器质量附加的耐力消耗 |
| `BaseStaminaCost` | 按武器类型读取的基础耐力消耗表 |

### Posture

| 键名 | 作用 |
| --- | --- |
| `UsePostureSystem` | 架势系统总开关 |
| `UsePostureHUD` | 是否输出架势 HUD，当前面向 TrueHUD |
| `MaxPostureHealthMult` | 每点最大生命带来的最大架势增量 |
| `MaxPostureStaminaMult` | 每点最大耐力带来的最大架势增量 |
| `PostureRegenDelay` | 架势恢复前的延迟 |
| `PostureRegenPercentPerSecond` | 每秒恢复的最大架势百分比 |
| `BashPostureDamageMult` | Bash 的架势伤害倍率 |
| `PowerAttackPostureDamageMult` | 重击的架势伤害倍率 |
| `PowerBashPostureDamageMult` | Power Bash 的架势伤害倍率 |
| `ArmorPostureDamageFactor` | 护甲减少架势伤害时使用的因子 |
| `BasePosture` | 按种族类型读取的基础最大架势表 |
| `BasePostureDamage` | 按武器类型读取的基础架势伤害表 |

### Poise

| 键名 | 作用 |
| --- | --- |
| `UsePoiseSystem` | 韧性系统总开关 |
| `PoiseStaminaMult` | 每点最大耐力提供的最大韧性增量 |
| `PoiseMassMult` | 装备质量带来的最大韧性增量 |
| `PoiseRegenDelay` | 韧性恢复前的延迟 |
| `PoiseRegenPercentPerSecond` | 每秒恢复的最大韧性百分比 |
| `StaggerLevelSmall` | 小硬直阈值 |
| `StaggerLevelMedium` | 中硬直阈值 |
| `StaggerLevelLarge` | 大硬直阈值 |
| `LightArmorHeadMaxPoiseBonus` | 轻甲头部提供的最大韧性加成 |
| `LightArmorBodyMaxPoiseBonus` | 轻甲身体提供的最大韧性加成 |
| `LightArmorHandMaxPoiseBonus` | 轻甲手部提供的最大韧性加成 |
| `LightArmorFeetMaxPoiseBonus` | 轻甲脚部提供的最大韧性加成 |
| `HeavyArmorHeadMaxPoiseBonus` | 重甲头部提供的最大韧性加成 |
| `HeavyArmorBodyMaxPoiseBonus` | 重甲身体提供的最大韧性加成 |
| `HeavyArmorHandMaxPoiseBonus` | 重甲手部提供的最大韧性加成 |
| `HeavyArmorFeetMaxPoiseBonus` | 重甲脚部提供的最大韧性加成 |
| `BashPoiseDamageMult` | Bash 的韧性伤害倍率 |
| `PowerAttackPoiseDamageMult` | 重击的韧性伤害倍率 |
| `PowerBashPoiseDamageMult` | Power Bash 的韧性伤害倍率 |
| `BasePoise` | 按种族类型读取的基础最大韧性表 |
| `BasePoiseDamage` | 按武器类型读取的基础韧性伤害表 |

### Stagger

| 键名 | 作用 |
| --- | --- |
| `UseStaggerSystem` | 硬直扩展层总开关 |
| `StaggerRecoveryTimeSmall` | 小硬直恢复时间 |
| `StaggerRecoveryTimeMedium` | 中硬直恢复时间 |
| `StaggerRecoveryTimeLarge` | 大硬直恢复时间 |

### Exhausted

| 键名 | 作用 |
| --- | --- |
| `UseExhaustedSystem` | 力竭系统总开关 |
| `DisablePlayerAttackWhenExhausted` | 玩家力竭时是否禁止攻击 |
| `DisableNPCAttackWhenExhausted` | NPC 力竭时是否禁止攻击 |
| `ExitExhaustedOnHit` | 被击中时是否退出力竭 |
| `ExhaustedExitPercent` | 退出力竭所需的耐力百分比 |
| `AttackDamageMultWhenExhausted` | 力竭时造成伤害倍率 |
| `AttackPostureDamageMultWhenExhausted` | 力竭时造成架势伤害倍率 |
| `OnHitDamageMultWhenExhausted` | 力竭时受到伤害倍率 |
| `OnHitPostureDamageMultWhenExhausted` | 力竭时受到架势伤害倍率 |

### Block

| 键名 | 作用 |
| --- | --- |
| `UseBlockSystem` | 格挡系统总开关 |
| `TimedBlockEnabled` | 是否启用限时格挡 |
| `TimedBlockNeverPostureBreak` | 限时格挡时是否允许完全免疫破防 |
| `TimedBlockLimit` | 限时格挡判定窗口 |
| `TimedBlockDuration` | 限时格挡收益持续时间 |
| `BlockMaxStaminaConsumePercent` | 单次格挡最多消耗的耐力百分比 |
| `BlockMinStaminaConsume` | 耐力不足时附加的最低耐力消耗 |
| `TimedBlockBlockStrengthMult` | 限时格挡期间的额外格挡强度倍率 |
| `BlockStrength` | 按武器类型读取的格挡强度表 |

### WeaponArt

| 键名 | 作用 |
| --- | --- |
| `UseWeaponArtSystem` | 战技系统总开关 |
| `UseWeaponArtHUD` | 是否启用战技 HUD |
| `WeaponArtHUDPosX` | HUD 横向位置，按屏幕百分比 |
| `WeaponArtHUDPosY` | HUD 纵向位置，按屏幕百分比 |
| `WeaponArtHUDScale` | HUD 缩放 |
| `WeaponArtMenuStartPercent` | 战技分配菜单起始横向位置 |

### Execution

| 键名 | 作用 |
| --- | --- |
| `UseExecutionSystem` | 处决逻辑总开关 |
| `OnHitDamageMultWhenExecutable` | 对可处决目标命中时的伤害倍率 |
| `ExecutionDamageMult` | 按武器类型读取的处决伤害倍率表 |

---

## 6. Modder 说明：动画事件与 Payload

这部分是给 Modder、动画作者和内容制作者看的，普通玩家不需要关心。

### 基本规则

- 动画事件标签和 payload 字符串在进入路由前都会被统一转成小写。
- 一次性战斗缓存会在 `attackstop` 和 `staggerstart` 时自动清理。
- 原生侧已经监听了原版和框架事件，例如 `weaponswing`、`weaponleftswing`、`collision_attackstart`、`collision_add`、`collision_attackend`、`blockstart`、`blockstartout`、`blockstop`。

### Rim Combat 自定义事件

| 事件标签 | Payload 语法 | 作用 |
| --- | --- | --- |
| `RimStamina` | `Start` | 让当前动作进入 Rim Combat 的耐力处理模式 |
| `RimStamina` | `End` | 结束当前动作的自定义耐力处理 |
| `RimStamina` | `Consume|AttackType|Side|Multiplier|FallbackMultiplier` | 按攻击类型和左右手信息结算耐力消耗 |
| `RimDamage` | `SetMult|Multiplier|FallbackMultiplier` | 设置当前近战动作的伤害倍率 |
| `RimDamage` | `End` | 清理当前动作缓存的伤害倍率 |
| `RimPoise` | `Set|Multiplier` | 设置自身承受韧性伤害的倍率 |
| `RimPoise` | `End` | 清理自身韧性倍率 |
| `RimPoise` | `TargetSet|Multiplier` | 设置目标承受韧性伤害的倍率 |
| `RimPoise` | `TargetEnd` | 清理目标韧性倍率 |
| `RimPosture` | `Unbreakable|Duration` | 让角色在一定毫秒数内免疫架势崩溃 |
| `RimPosture` | `Damage|Multiplier|FallbackMultiplier` | 设置当前动作的架势伤害倍率 |
| `RimPosture` | `End` | 清理当前动作缓存的架势倍率 |
| `RimStagger` | `TargetSet|Level` | 设置本次命中要施加的硬直等级 |
| `RimStagger` | `TargetEnd` | 清理命中目标的硬直等级缓存 |
| `RimStagger` | `Immune|Level|Duration` | 让角色在一定毫秒数内免疫指定等级及以下的硬直 |
| `RimWeaponArt` | `Start|ManaCost|MinMana` | 开始战技动作，记录是否满足施放条件，并启用 Rim Combat 耐力处理 |
| `RimWeaponArt` | `End` | 结束当前战技动作，并清理相关缓存 |
| `RimWeaponArt` | `PrepareEnd` | 把战技状态从准备中切到已启用 |
| `RimWeaponArt` | `ToPrepare` | 强制把战技状态切到准备中 |
| `RimWeaponArt` | `Cast|SpellName` | 施放当前战技里配置好的法术条目 |
| `RimExecution` | `Damage|Multiplier` | 实验性的处决伤害事件 |
| `RimExecution` | `VictimEnd` / `End` | 实验性的处决结束事件 |

### 当前需要注意的一点

- 硬直枚举里已经包含 `Knockdown`、`Strikefly`、`Knockaway` 这些额外等级。
- `TargetSet|Level` 和 `Immune|Level|Duration` 都可以使用这些额外等级。
- 较弱的 `TargetSet|Level` 不会再把当前更强的硬直等级压低。
- 同等级硬直重复命中时，会刷新恢复时间，但不会强制重播同一级硬直动画。

### 战技 JSON 编写说明

战技文件位于 `Data/SKSE/Plugins/RimCombat/WeaponArt/`。

| 字段 | 是否必须 | 说明 |
| --- | --- | --- |
| 对象键名 | 是 | 该键名会被哈希成运行时战技 ID |
| `name` | 是 | UI 使用的显示名 |
| `description` | 否 | UI 描述 |
| `availableWeapon` | 否 | `AvailableWeapon` 枚举名数组 |
| `weapons` | 否 | 显式武器列表，主要给 `Unique` 战技使用 |
| `consumePoint` | 否 | 解锁需要消耗的战技点数 |
| `unlockLevel` | 否 | 解锁所需战技等级 |
| `needPrepare` | 否 | 是否需要先进入准备态 |
| `spells` | 否 | `Cast|SpellName` 使用的法术条目定义 |
| `verbose` | 否 | 是否把加载信息输出到日志 |

当前匹配规则：

- `availableWeapon` 没写重量标签时，会被当成重量通配。
- `availableWeapon` 没写武器家族标签时，会被当成家族通配。
- `availableWeapon` 没写攻击方式标签时，攻击类型匹配会按全匹配处理。
- `Unique` 用于显式指定武器，应搭配 `weapons` 列表一起使用。
- `spells` 的键名在哈希前会转成小写，所以 `Cast|SpellName` 在大小写上不敏感，但逻辑键必须一致。

最小示例：

```json
{
  "Vacuum Slash": {
    "name": "Vacuum Slash",
    "description": "向前斩出并附带投射派生。",
    "availableWeapon": ["Normal", "Heavy", "Slash"],
    "consumePoint": 2,
    "unlockLevel": 0,
    "needPrepare": false,
    "spells": {
      "VacuumChop": {
        "mod": "EldenSkyrim_RimSkills.esp",
        "formID": "0xBA21",
        "effectiveness": 1.0,
        "stdMagnitude": 25.0,
        "factor": 10.0,
        "skill": "OneHanded",
        "selfCast": false
      }
    }
  }
}
```

### 远程 / 法术硬直 JSON

硬直映射文件位于 `Data/SKSE/Plugins/RimCombat/Stagger/`，格式是对象数组。

示例：

```json
[
  {
    "mod": "EldenSkyrim_RimSkills.esp",
    "formID": "0xB3C5",
    "level": "Strikefly"
  }
]
```

这可以让你在不改 C++ 的情况下，把某个投射物或法术命中映射到特定硬直等级。