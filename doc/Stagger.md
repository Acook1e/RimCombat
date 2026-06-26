# Stagger · 硬直系统

## 定位

统一管理 Skyrim 的受击动画系统。将原版混乱的 17 种硬直类型归一化为四级通用硬直 + 七种特殊硬直。通过 Modern Stagger Lock (MSL) 分发动画，用图变量控制恢复和免疫。

---

## 硬直等级枚举

| 等级 | 枚举值 | 说明 |
|------|:---:|------|
| `None` | 0 | 未硬直 |
| `Small` | 1 | 轻硬直 |
| `Medium` | 2 | 中硬直 |
| `Large` | 3 | 重硬直 |
| `Largest` | 4 | 最重硬直（以上为特殊等级） |
| `Knockaway` | 100 | 击飞 |
| `Knockdown` | 101 | 砸趴 |
| `Strikefly` | 102 | 挑飞 |
| `GuardBreak` | 200 | 防御崩溃（格挡耐力归零 / 弹反成功） |
| `Executor` | 253 | 处决中 — 处决者动画硬直 |
| `Execution` | 254 | 处决中 — 受害者动画硬直 |
| `PostureBreak` | 255 | 架势崩溃 |

### 等级优先级

| 规则 | 行为 |
|------|------|
| Normal (< Largest) 之间 | 高覆盖低，大硬直期间小硬直忽略，同级别不重复触发 |
| Special (> Largest) 盖 Normal | 特殊硬直不被普通硬直打断（除 PostureBreak 可被普通硬直覆盖） |
| 最高优先级 | `GuardBreak`、`Executor`、`Execution`、`PostureBreak` — 无法被免疫 |
| Normal 免疫 | `IsImmune()` 检测：免疫持续时间到 / 不在免疫列表中 |

---

## 硬直方向

| 枚举值 | 含义 |
|:---|------|
| `Front` | 从正面击中 → MSL 播 Forward 变体 |
| `Back` | 从背面击中 → MSL 播 Backward 变体 |

---

## MSL 集成

| 图变量 | 用途 |
|------|------|
| `RimCombat_StaggerLevel` | 当前硬直等级 — OAR 条件匹配 |
| `MSL_IsStaggerRecovery` | 是否可恢复 — `true`=硬直结束中 |
| `MSL_StaggerMagnitude` | 硬直大小 — MSL 用来选择动画强度 |
| `MSL_StaggerDirection` | 硬直方向 — MSL 选 Forward/Backward 变体 |

**Magnitude 映射**：MSL 的 ini 文件定义四种常规硬直的 magnitude 阈值（0.25/0.5/0.75/1.0），`Stagger::Constructor` 解析 ini 并缓存到 `staggerMagnitudeMap`。所有 ≥ `Largest` 的特殊硬直映射到 1.0。

---

## 硬直生命周期

```
SetStaggerLevel → ProcessWeaponStagger → StaggerStart
  ├─ 首次硬直: IsStaggering=false → 发 staggerStart 动画事件 → 写恢复计时
  ├─ 升级硬直: IsStaggering=true → 发 staggerStop(停旧) → 发 staggerStart(开新)
  └─ 降级/同级: 跳过，不重启动画

恢复路径 (三选一):
  ├─ 自然恢复: 恢复计时到期 → MSL_IsStaggerRecovery=true → 动画自然结束
  ├─ 动画事件: staggerStop 触发 → Recoverable() → 立即清除
  └─ 动画 Payload: RimStagger → recoverable → 立即清除
```

---

## 动画事件

通过 `RimStagger` 图事件控制硬直状态：

| Payload | 效果 |
|------|------|
| `TargetSet\|Level` | 强制写入硬直等级（绕过韧性计算） |
| `TargetEnd` | 清除 TargetSet 写入的等级 |
| `Immune\|Duration` | 免疫正常硬直 `Duration` ms |
| `Recoverable` | 立即恢复硬直 |

---

## 弹射物硬直

弩箭 / 法术投射物命中时，通过 `Stagger/EldenRim.json` 配置的 FormID → 硬直等级映射表决定硬直。每条记录包含 `mod`（插件名）、`formID`（十六进制）、`level`（锁定的硬直等级枚举名）。

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUseStaggerSystem` | 总开关 |
| `uStaggerRecoveryTimeSmall` | 轻硬直恢复时间 (ms) |
| `uStaggerRecoveryTimeMedium` | 中硬直恢复时间 (ms) |
| `uStaggerRecoveryTimeLarge` | 重硬直恢复时间 (ms) |
