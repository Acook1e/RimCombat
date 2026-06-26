# Posture · 架势系统

## 定位

"破防"资源池 — 类似架势条的独立血量。每次攻击削减受害者架势值，归零触发 `PostureBreak` 硬直并将受害者标记为**可处决**。架势自动再生。

---

## 架势上限计算

```
架势上限 = 种族基础值
         + 生命上限 × MaxPostureHealthMult (0.06)
         + 耐力上限 × MaxPostureStaminaMult (0.04)
```

种族基础值通过 `Posture/Skyrim.json` 的 `BasePosture` 配置，可按 NPC FormID 覆盖。默认 Human=100，Dragon=220，Giant=188。

---

## 伤害管线

武器命中 → `ProcessWeaponHit` → 计算姿势伤害 → `DamagePostureHealth`：

```
伤害 = 基础架势伤害值
     × 攻击类型倍率（普通/强力/盾击/强盾）
     × 战技动画缓存倍率（TargetSet）
     → (如格挡中) 转交 Block::ProcessPostureDamage 处理
     → 护甲递减（指数衰减, ArmorPostureDamageFactor = 1.1）
     → 力竭倍率（攻击方/受击方）
```

### 破损逻辑

| 条件 | 行为 |
|------|------|
| 架势归零 | 发送 `PostureBreak` 硬直 (255)，标记为可处决 |
| 破损后 | 架势恢复到最大值的 80%，避免连续破防 |
| 力竭时防破 | 攻击方力竭时架势伤害减半，受击方力竭时受到的架势伤害增加 |

---

## 再生

- **再生延迟**：最后一次受伤 `PostureRegenDelay` ms（默认 5000ms）后开始再生
- **再生速率**：每秒恢复 `PostureRegenPercentPerSecond`%（默认 3%）
- **处决中**：再生减速至 20%（每 5000ms 恢复正常 1000ms 的量）
- **起始值**：处决开始时架势从 50% 最大值恢复

---

## TrueHUD 可视化

架势条通过 TrueHUD 的特殊栏（special bar）显示：

- 显示当前/最大架势值
- 破防后闪烁/变色提示
- 再生期间颜色渐变

---

## 动画事件

通过 `RimPosture` 图事件控制：

| Payload | 效果 |
|------|------|
| `TargetSet\|mult` | 设置动画缓存的攻击方架势倍率，单次有效 |
| `Unbreakable\|duration` | 免疫架势归零，持续 `duration` ms |
| `TargetEnd` | 清除缓存 |

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUsePostureSystem` | 总开关 |
| `MaxPostureHealthMult` | 生命系数（默认 0.06） |
| `MaxPostureStaminaMult` | 耐力系数（默认 0.04） |
| `PostureRegenDelay` | 再生延迟 (ms) |
| `PostureRegenPercentPerSecond` | 再生速率（%/秒） |
| `ArmorPostureDamageFactor` | 护甲递减指数 |
| `BashPostureDamageMult` | 盾击架势倍率 |
| `PowerAttackPostureDamageMult` | 强力攻击架势倍率 |
| `PowerBashPostureDamageMult` | 强力盾击架势倍率 |
| `BasePosture` | 种族基础值 (`Posture/Skyrim.json`) |
| `BasePostureDamage` | 武器类型基础架势伤害值 |
