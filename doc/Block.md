# Block · 格挡系统

## 定位

重建原版格挡为三层结构：普通格挡 → 限时格挡 → 精确格挡 (GP) / 弹反 (Parry)。Bash 和 Power Bash 的伤害判定完全移除，转为纯格挡/反击功能动作。

---

## 三层格挡

### 1. 普通格挡

- 启动：`blockStart` / `blockStartOut` 动画事件
- 格挡强度：每种武器类型独立（`blockStrengthMap`），重量武器更高
- 耐力消耗：被击时消耗 `攻击伤害 × fBlockMaxStaminaConsumePercent`，最低不低于 `fBlockStaminaBonus`
- 架势保护：格挡中的架势伤害转交 `Block::ProcessPostureDamage` 处理

### 2. 限时格挡 (Timed Block)

格挡开始后 150ms (`uTimedBlockLimit`) 内有效。

| 特性 | 值 |
|------|:---:|
| 窗口大小 | 150ms |
| 有效持续时间 | 100ms |
| 格挡强度倍率 | 4.0× (`fTimedBlockBlockStrengthMult`) |
| 架势保护 | 完全免疫架势崩溃 (`bTimedBlockNeverPostureBreak`) |

超时后自动回退为普通格挡。

### 3. 精确格挡 / 弹反

| 动作 | 原版 | RimCombat | 触发方式 |
|------|------|------|------|
| **Bash** | 盾击（伤害+削韧） | **GP 动作**（纯防守，无伤害） | 玩家按键 Bash |
| **Power Bash** | 强力盾击 | **弹反**（成功给予 `GuardBreak` 硬直） | 玩家按键 Power Bash |

- **GP**：Bash 命中瞬间生成 `GPData`（窗口时长 + 等级）。GP 成功后可接自动反击（`GP|Duration|Level|Auto|Next` payload）。
- **弹反**：Power Bash 成功时给攻击者发送 `GuardBreak` 硬直（200），攻击者的 PostureBreaks 后强制格挡耐力归零。
- 视觉/音效：GP 成功时播放火花粒子特效 + 专用 WAV。

---

## 动画事件

通过 `RimBlock` 图事件配置：

| Payload | 效果 |
|------|------|
| `GP\|Duration\|Level\|Auto\|Next` | 开启 GP 窗口 |
| `Parry\|Duration` | 开启弹反窗口 |

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUseBlockSystem` | 总开关 |
| `bTimedBlockEnabled` | 启用限时格挡 |
| `bTimedBlockNeverPostureBreak` | 限时格挡免疫架势破防 |
| `uTimedBlockLimit` | 限时窗口 (ms, 默认 150) |
| `uTimedBlockDuration` | 限时有效期 (ms, 默认 100) |
| `fBlockMaxStaminaConsumePercent` | 被击耐力消耗比例（默认 0.5） |
| `fBlockStaminaBonus` | 最低耐力消耗（默认 10.0） |
| `fTimedBlockBlockStrengthMult` | 限时格挡强度倍率（默认 4.0） |
| `BlockStrength` | 每种武器类型格挡强度 |
