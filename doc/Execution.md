# Execution · 处决系统

## 定位

架势崩溃后触发的终结技系统。受害者进入可处决池，处决者按攻击键自动锁定最近的有效受害者。完全验证 → 原子认领 → 强制定位 → 硬直触发 → 动画播段伤害 → 延迟杀死。

---

## 流程

```
PostureBreak → EnterExecutable(受害者)
                     ↓
玩家 ActionRightAttack → FindExecutableTarget(玩家)
  ├─ 距离 ≤ 250
  ├─ 玩家前方 60° 锥内
  ├─ 高度差 ≤ 50
  ├─ 武器|种族组合存在且方向可用
  └─ unique_lock 内原子 erase → 返回 (victim, direction)

Execute(玩家, victim, direction)
  ├─ AI 锁（处决者 kProcessMe=false）
  ├─ 图变量设置（ExecutorWeapon, VictimRace, MCO_nextattack）
  ├─ 硬直设置（aggressor→Executor, victim→Execution）
  ├─ 定位（victim 不动 → aggressor 移到 150 距离 → 设置朝向）
  ├─ attackStart 硬直（含失败重试）
  └─ 注册到 executingActors

动画播放中：
  ├─ RimExecution → damage|0.5  → 结算伤害（留 ≥ 0.5HP）
  ├─ RimExecution → damage|1.0  → 终结段
  └─ RimExecution → end          → ExecutionEnd → 杀死
```

---

## 方向系统

| 枚举值 | 含义 | OAR MCO_nextattack |
|:---|------|:---:|
| `Front` | 正面处决 | 1 |
| `Back` | 背刺 | 2 |

- **正面约定**：处决者面对受害者 → 受害者面对处决者（180° 对视）
- **背刺约定**：处决者面对受害者背部 → 二者面向相同
- **距离**：处决者移至受害者前方/后方 150 处。受害者不动（硬直中）
- **高度**：二者 Z 轴统一到更高者

---

## 动画类别（受害者端）

MSL 硬直动画替换，四个 OAR config 分组：

| 类别 | 武器 | 正向 | 背刺 |
|------|------|:---:|:---:|
| **Thrust** | Dagger, Sword, Rapier, Katana, ShortSpear | ✓ | ✓ |
| **Knock** | WarAxe, Mace, BattleAxe, WarHammer | ✓ | - |
| **TwoHand** | GreatSword, GreatKatana, Glaive, Pike, Halberd | ✓ | - |
| **Fist** | Unarm, Spell, Claw, Cestus | ✓ | - |

OAR 条件：`RimCombat_StaggerLevel == 254` + `ExecutorWeapon` + `VictimRace`。

---

## 伤害管线

```
总伤害 = 武器基础攻击力 × ExecutionDamageMult × animationPayload

- 武器基础攻击力 = 武器面板 (GetAttackDamage) + 空手攻击力
- ExecutionDamageMult = Settings.json 中该武器类型的倍率
- animationPayload = RimExecution → damage|N 中的 N
```

| 类别 | 武器 mult 范围 | 动画 payload 合计 | 总伤害（以 Sword 为例） |
|------|:---:|:---:|:---:|
| Thrust | 2.3 - 3.0 | 2.0 | ~55 |
| Knock | 1.3 - 2.5 | 1.8 | ~54 |
| TwoHand | 1.5 - 1.9 | 2.2 | ~73 |
| Fist | 1.5 - 2.0 | 1.8 | ~30 |

### 延迟杀死

- 每段 damage payload 保证受害者血量 ≥ 0.5HP（`current - 0.5`）
- `RimExecution → end` 时检测血量 < 1HP → 执行 `DamageActorValue(kHealth, 1.0)` 杀死
- 允许动画 end 事件和处决者 Executor 硬直的自然 staggerStop 双路径触发

---

## 无敌保护

| 机制 | 对象 | 方式 |
|------|------|------|
| IsGhost detour | 受害者 | 外部弹射物/近战命中穿透，不可选中 |
| 伤害 hook 过滤 | 处决者 + 受害者 | `ProcessHit` 中检查 `IsExecutingActor()` → 跳出 |

---

## 动画事件

通过 `RimExecution` 图事件控制：

| Payload | 效果 |
|------|------|
| `damage\|N` | 结算伤害，倍率 N |
| `end` | 结束处决，触发杀死 + 清理 |

---

## JSON 注册表

- **文件**：`Execution/RimCombat.json`
- **结构**：Race-first — `{ <Race>: { <Weapon>: ["Front", "Back"] } }`
- **运行时**：构造时加载 → `availableExecutions`（`map<Weapon\|Race, Direction>`），查找 O(1)

---

## 图变量

| 变量 | 用途 |
|------|------|
| `RimCombat_ExecutorWeapon` | 处决者武器 Int 枚举值 |
| `RimCombat_VictimRace` | 受害者种族 Int 枚举值 |
| `MCO_nextattack` | 方向：1=正面，2=背刺 |

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUseExecutionSystem` | 总开关 |
| `fOnHitDamageMultWhenExecutable` | 可处决状态下的受击伤害倍率 |
| `ExecutionDamageMult` | 每种武器类型的处决伤害倍率（23 条目） |
| `ExitExecutionOnHit` | 被攻击时退出处决 |
| `ExecutableDuration` | 可处决状态持续时间 (ms) |
