# Poise · 韧性系统

## 定位

管理"攻击者能否打出硬直"的再生资源池。每次受击扣除韧性值，低于硬直阈值时触发对应等级硬直。韧性值持续自动恢复。

---

## 韧性上限计算

```
韧性上限 = 种族基础值
         + 当前耐力 × fPoiseStaminaMult
         + 装备总质量 × fPoiseMassMult
         + 头盔护甲加成 + 手套护甲加成 + 身体护甲加成 + 靴子护甲加成
```

四个护甲部位按**轻甲 / 重甲**分别有独立加成值。`Settings.json` 中为 `LightArmorPoise[Slot]` / `HeavyArmorPoise[Slot]` 各四个字段。

种族基础值可通过 `Poise/Skyrim.json` 中的 `BasePoise` 覆盖，或按 NPC 的 FormID 在 `NPC` 字段中覆盖。

---

## 伤害管线

武器命中 → `ProcessWeaponHit` → 计算最终韧性伤害：

```
伤害 = 基础削韧值
     × Perk 修正 (ModTargetStagger / ModIncomingStagger, 钳制 0.6~2.0)
     × 等级衰减因子
     × 攻击类型倍率 (普通 / 强力 / 盾击 / 强盾)
     × 动画缓存倍率 (TargetSet)
     - 出手韧性抵扣
```

### 各子系统

| 环节 | 说明 |
|------|------|
| **基础削韧值** | 从 `basePoiseDamageMap` 读取攻击者武器类型的削韧值。生物攻击按种族读取 `baseCreaturePoiseDamage` |
| **Perk 修正** | `kModTargetStagger`（攻击方）和 `kModIncomingStagger`（受击方），最终值钳制在 [0.6, 2.0] |
| **等级衰减** | 仅攻击者等级低于受击者时生效：`1 ÷ (1 + 等级差 × fLevelDiffAggressorMultPerLevel)`。高打低无衰减 |
| **动画缓存倍率** | `RimPoise → TargetSet\|mult` 事件在动画时间线设置，单次有效，攻击后自动清除 |
| **出手韧性** | 攻击中受击时，抵扣 `自身削韧基础值 × fVictimAttackingPoiseBonusPercent`（默认 80%）。最低降为原伤害的 10% — 重武器出手时几乎不会被轻攻击打断 |

---

## 硬直触发

攻击造成韧性伤害后，调用 `CalculateStaggerLevel` 判定：

| 韧性伤害 ≥ | 硬直等级 |
|:---|------|
| 2.0 | `Small` |
| 5.0 | `Medium` |
| 9.0 | `Large` |

达到 `Largest`（1.0× 韧池总容量）由 Modern Stagger Lock 的 magnitude 判定，不在代码层。阈值可通过 `Settings.json`（`fStaggerLevelSmall/Medium/Large`）调整。

**连续硬直补偿**：若上一次硬直恢复中途再次触发同等级硬直，新硬直时间按 `(剩余时间 + 新时间) × 20%` 缩短。大硬直期间的小硬直被忽略。

---

## 动画事件

通过 `RimPoise` 图事件，动画可临时覆盖韧性倍率：

| Payload | 效果 |
|------|------|
| `Set\|mult` | 设置**防御方**倍率（影响此后被击的韧性伤害） |
| `TargetSet\|mult` | 设置**攻击方**倍率（影响此后攻击的削韧值） |
| `End` | 清除防御方缓存 |
| `TargetEnd` | 清除攻击方缓存 |

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUsePoiseSystem` | 总开关 |
| `fPoiseStaminaMult` | 耐力系韧性系数 |
| `fPoiseMassMult` | 质量系韧性系数 |
| `uPoiseRegenDelay` | 受击后再生延迟（ms） |
| `fPoiseRegenPercentPerSecond` | 每秒恢复最大值的百分比 |
| `fStaggerLevelSmall/Medium/Large` | 三级硬直阈值 |
| `fStaggerCompensationPercent` | 连续硬直补偿比例 |
| `fBashPoiseDamageMult` | 盾击削韧倍率 |
| `fPowerAttackPoiseDamageMult` | 强力攻击削韧倍率 |
| `fPowerBashPoiseDamageMult` | 强力盾击削韧倍率 |
| `fLevelDiffAggressorMultPerLevel` | 每级削韧衰减系数 |
| `fVictimAttackingPoiseBonusPercent` | 出手韧性比例 |
| `LightArmorPoise/HeavyArmorPoise` | 四个护甲部位独立加成 |
| `BasePoise` | 种族基础值（`Poise/Skyrim.json`） |
| `BasePoiseDamage` | 武器类型基础削韧值 |
