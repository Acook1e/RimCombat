# Stamina · 耐力系统

## 定位

完全接管原版攻击耐力消耗，替换为**按武器类型 × 种族 × 攻击类型**的分类消耗模型。零耐力时禁止攻击。力竭（Exhausted）状态进一步改变攻防倍率。

---

## 消耗通道（5 条入口）

| 通道 | 触发条件 | 适用对象 |
|------|---------|---------|
| **Swing** | `kSwing` 攻击状态帧检测 | 人型/Draugr/Falmer 种族持武器攻击 |
| **Creature** | `kSwing` 检测，非武器种族 | 熊、龙、蜘蛛等无法装备武器的生物 |
| **Bash** | `kBash` 攻击状态帧检测 | 所有盾击 / 强力盾击 |
| **Unarm** | 动画事件 `soundplay.wpnunarmedswing` | 空手攻击（Swing 状态对空手不可靠） |
| **RimStamina** | 动画事件 `RimStamina\|Consume\|AT\|Side\|Mult` | 战技驱动的动画注解消费 |

**优先级规则**：如果 actor 当前在 `RimStamina` 通道的活跃集合中（由 `Start`/`End` 管理），则前四个检测型通道全部旁路，完全委托给动画事件通道。

---

## 消耗公式

```
消耗 = 基础值 × 质量系数 × 攻击类型系数
```

- **基础值**：`(武器类型 → baseStaminaCostMap)` 或 `(生物种族 → baseCreatureStaminaMap)`
- **质量系数**：`武器重量 × 每质量消耗常数`（普通攻击）/ `武器重量 × 强力攻击每质量消耗常数`（强力攻击）
- **攻击类型系数**：
  - 普通攻击 → `1.0`
  - 强力攻击 → `fPowerAttackStaminaCostMult`
  - 盾击 → `fBashStaminaCostMult`
  - 强力盾击 → `fPowerBashStaminaCostMult`
- **Perk 修正**：`kModPowerAttackStamina` perk entry point 仍然生效

### 武器侧 (Side) 解析

| 枚举值 | 行为 |
|:---|------|
| `None` | 无消耗 |
| `Left` | 查询左手装备类型 |
| `Right` | 查询右手装备类型 |
| `Creature` | 按生物种族查询 |
| `Auto` | 自动判断：武器种族 → 右手武器类型，生物种族 → 种族类型 |

---

## 力竭系统 (Exhausted)

当耐力降至 0 时进入力竭状态。

| 参数 | 默认值 | 说明 |
|------|:---:|------|
| 退出条件 | 耐力恢复到 `max × 20%` | `ExhaustedExitPercent` |
| 受击退出 | 开启 | `ExitExhaustedOnHit` — 被击中时强制退出 |
| 攻击伤害倍率 | 0.7× | `AttackDamageMultWhenExhausted` |
| 受击伤害倍率 | 1.5× | `OnHitDamageMultWhenExhausted` |
| 架势伤害倍率 | 乘上法相同参数 | 攻击/受击各有独立倍率 |
| TrueHUD 反馈 | 灰屏叠加 | 进入力竭时 `EnterGreyOut`，退出时 `ExitGreyOut` |

力竭状态在存档中通过 SKSE 序列化持久化。

---

## 攻击禁用

当玩家或 NPC 耐力 = 0 时，直接禁用攻击：

- **玩家**：`ActionRightAttack` / `ActionLeftAttack` 钩子返回 `false`，让输入事件不传播
- **NPC**：`GetAttackData` 钩子阻断攻击数据生成
- **恢复条件**：耐力 > 0 时自动恢复攻击能力

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUseAttackStaminaSystem` | 总开关 |
| `bDisableAttackWhenOutOfCombat` | 战斗中才消耗耐力 |
| `bDisableNPCStaminaConsume` | NPC 免耐力消耗 |
| `bDisablePlayerAttackWhenStaminaEmpty` | 零耐力禁玩家攻击 |
| `bDisableNPCAttackWhenStaminaEmpty` | 零耐力禁 NPC 攻击 |
| `fAttackStaminaCostPerMass` | 普通攻击每质量消耗 |
| `fPowerAttackStaminaCostPerMass` | 强力攻击每质量消耗 |
| `fPowerAttackStaminaCostMult` | 强力攻击倍率 |
| `fBashStaminaCostMult` / `fPowerBashStaminaCostMult` | 盾击倍率 |
| `fStaminaRegenMult` / `fStaminaRegenCombatMult` / `fStaminaRegenBlockMult` | 恢复速率 |
| `BaseStaminaCost` | 每种武器类型基础消耗（25+ 条目） |
| `BaseCreatureStamina` | 每种生物种族基础消耗（50+ 条目） |
