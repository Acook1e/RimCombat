# Rim Combat · 处决自定义指南

> 本文指导用户添加新的处决组合、调试处决行为，以及理解为什么必须以 JSON 注册。

---

## 为什么必须注册处决

处决不是"任意种族 + 任意武器 + 任意方向"都能触发的。系统内部使用以下链路判断是否允许：

```
玩家按键 → FindExecutableTarget
  → 查 executableActors（受害者在池中？）
  → 查 availableExecutions[weapon | race]（注册的组合存在？）
  → 查 direction > needed（该方向允许？）
  → 原子认领 victim → Execute
```

**如果没有在 `Execution/RimCombat.json` 中注册 `(种族, 武器, 方向)` 组合，`availableExecutions` 的 hash 表不会包含该条目。** 即使受害者已经在可处决池中、距离角度都满足，也会在"组合不存在"这一步被跳过 — 攻击键按下后什么都不发生。

注册是唯一的准入机制。不要靠猜或默认值 — 必须显式写入 JSON。

---

## 正面 / 背刺动画

处决有两套动画：

### 受害者端（MSL 硬直替换动画）

文件放在：<br>
`Meshes/.../RimCombat - Stagger/Execution - <种族> - <类别>/ModernStaggerLock/`

| 文件名 | 含义 |
|------|------|
| `MSL_StaggerForwardLargest.hkx` | **正面**处决 — 受害者面朝处决者的受击动画 |
| `MSL_StaggerBackwardLargest.hkx` | **背刺** — 受害者背对处决者的受击动画 |

MSL 根据 `StaggerDirection` 图变量自动选择 Forward 还是 Backward。如果文件夹内只有 Forward 没有 Backward，该处决类别只能正面触发 — `Execution/RimCombat.json` 中不应注册 `"Back"` 方向。

### 处决者端（MCO/BFCO 攻击动画）

处决者通过 `attackStart` 事件触发攻击动画。OAR 子集条件使用以下图变量：

| 图变量 | 正面 | 背刺 |
|------|:---:|:---:|
| `MCO_nextattack` | 1 | 2 |

---

## 完整种族枚举

| Int 值 | 枚举名 | 中文 |
|:---:|------|------|
| 0 | `None` | 无 |
| 1 | `Human` | 人类 |
| 2 | `AshHopper` | 灰烬跳跃者 |
| 3 | `Bear` | 熊 |
| 4 | `Boar` | 野猪 |
| 5 | `BoarMounted` | 骑乘野猪 |
| 6 | `Chaurus` | 查鲁斯 |
| 7 | `ChaurusHunter` | 查鲁斯猎手 |
| 8 | `ChaurusReaper` | 查鲁斯收割者 |
| 9 | `Chicken` | 鸡 |
| 10 | `Cow` | 牛 |
| 11 | `Deer` | 鹿 |
| 12 | `Dog` | 狗 |
| 13 | `Dragon` | 龙 |
| 14 | `DragonPriest` | 龙祭司 |
| 15 | `Draugr` | 尸鬼 |
| 16 | `DwarvenBallista` | 矮人弩车 |
| 17 | `DwarvenCenturion` | 矮人百夫长 |
| 18 | `DwarvenSphere` | 矮人球 |
| 19 | `DwarvenSpider` | 矮人蜘蛛 |
| 20 | `Falmer` | 雪精灵 |
| 21 | `FlameAtronach` | 火焰侍灵 |
| 22 | `Fox` | 狐狸 |
| 23 | `FrostAtronach` | 冰霜侍灵 |
| 24 | `Gargoyle` | 石像鬼 |
| 25 | `Giant` | 巨人 |
| 26 | `GiantSpider` | 巨型蜘蛛 |
| 27 | `Goat` | 山羊 |
| 28 | `Hagraven` | 乌鸦鬼婆 |
| 29 | `Hare` | 野兔 |
| 30 | `Horker` | 三牙海象 |
| 31 | `Horse` | 马 |
| 32 | `IceWraith` | 冰霜怒灵 |
| 33 | `LargeSpider` | 大型蜘蛛 |
| 34 | `Lurker` | 潜伏者 |
| 35 | `Mammoth` | 猛犸 |
| 36 | `Mudcrab` | 泥沼蟹 |
| 37 | `Netch` | 水母 |
| 38 | `Riekling` | 蓝客灵 |
| 39 | `Sabrecat` | 剑齿虎 |
| 40 | `Seeker` | 寻觅者 |
| 41 | `Skeever` | 雪鼠 |
| 42 | `Slaughterfish` | 食人鱼 |
| 43 | `Spider` | 蜘蛛 |
| 44 | `Spriggan` | 树精 |
| 45 | `StormAtronach` | 风暴侍灵 |
| 46 | `Troll` | 巨魔 |
| 47 | `VampireLord` | 吸血鬼大君 |
| 48 | `Werebear` | 熊人 |
| 49 | `Werewolf` | 狼人 |
| 50 | `Wisp` | 幽魂之母碎片 |
| 51 | `Wispmother` | 幽魂之母 |
| 52 | `Wolf` | 狼 |

> 当前 0.4.0 仅支持 `Human` 的受害者动画。后续版本会为其他种族添加。

---

## 完整武器枚举

| Int 值 | 枚举名 | 中文 | 类别 |
|:---:|------|------|------|
| 1 | `Unarm` | 空手 | Fist |
| 2 | `Spell` | 法术 | Fist |
| 10 | `Dagger` | 匕首 | Thrust |
| 11 | `Claw` | 爪 | Fist |
| 20 | `Sword` | 单手剑 | Thrust |
| 21 | `Rapier` | 细剑 | Thrust |
| 22 | `Katana` | 武士刀 | Thrust |
| 30 | `WarAxe` | 单手斧 | Knock |
| 40 | `Mace` | 单手锤 | Knock |
| 41 | `Cestus` | 拳套 | Fist |
| 42 | `Whip` | 鞭 | — |
| 50 | `ShortSpear` | 短枪 | Thrust |
| 60 | `GreatSword` | 大剑 | TwoHand |
| 61 | `GreatKatana` | 大太刀 | TwoHand |
| 70 | `BattleAxe` | 战斧 | Knock |
| 80 | `WarHammer` | 战锤 | Knock |
| 81 | `Quarterstaff` | 长棍 | TwoHand |
| 90 | `Glaive` | 长柄刀 | TwoHand |
| 91 | `Pike` | 长枪 | TwoHand |
| 92 | `Halberd` | 斧枪 | TwoHand |
| 100 | `Bow` | 弓 | — |
| 101 | `Crossbow` | 弩 | — |
| 200 | `Staff` | 法杖 | — |
| 201 | `Shield` | 盾牌 | — |
| 202 | `Torch` | 火把 | — |

> OAR config 的 `RimCombat_ExecutorWeapon` 图变量使用上面 Int 值。远程武器（Bow/Crossbow）不参与处决。

---

## 0.4.0 默认注册表

当前 `Execution/RimCombat.json` 中的注册：

```jsonc
{
    "Human": {
        "Dagger":      ["Front","Back"],
        "Sword":       ["Front","Back"],
        "Rapier":      ["Front","Back"],
        "Katana":      ["Front","Back"],
        "ShortSpear":  ["Front","Back"],
        "WarAxe":      ["Front"],
        "Mace":        ["Front"],
        "BattleAxe":   ["Front"],
        "WarHammer":   ["Front"],
        "GreatSword":  ["Front"],
        "GreatKatana": ["Front"],
        "Glaive":      ["Front"],
        "Pike":        ["Front"],
        "Halberd":     ["Front"],
        "Unarm":       ["Front"],
        "Spell":       ["Front"],
        "Claw":        ["Front"],
        "Cestus":      ["Front"]
    }
}
```

---

## 如何添加新种族处决

### 1. 注册 JSON

在 `Execution/RimCombat.json` 的顶层对象中添加新种族条目：

```jsonc
{
    "Human": { ... },
    "Draugr": {
        "Sword":       ["Front"],
        "GreatSword":  ["Front"]
    }
}
```

### 2. 添加受害者动画（MSL 替换）

创建文件夹：<br>
`Meshes/.../RimCombat - Stagger/Execution - Draugr - OneHand/ModernStaggerLock/`

放入 `MSL_StaggerForwardLargest.hkx`（正面），如有背刺再放入 `MSL_StaggerBackwardLargest.hkx`。

### 3. 写入 OAR config.json

在同一文件夹下创建 `config.json`，仿 Thrust 模板：

```jsonc
{
  "name": "Execution - Draugr - OneHand",
  "description": "None",
  "priority": 2064143302,
  "conditions": [
    {
      "condition": "CompareValues",
      "requiredVersion": "1.0.0.0",
      "Value A": {
        "graphVariable": "RimCombat_StaggerLevel",
        "graphVariableType": "Int"
      },
      "Comparison": "==",
      "Value B": { "value": 254 }
    },
    {
      "condition": "OR",
      "requiredVersion": "1.0.0.0",
      "Conditions": [
        {
          "condition": "CompareValues",
          "requiredVersion": "1.0.0.0",
          "Value A": { "graphVariable": "RimCombat_VictimRace", "graphVariableType": "Int" },
          "Comparison": "==",
          "Value B": { "value": 15 }
        }
      ]
    },
    {
      "condition": "OR",
      "requiredVersion": "1.0.0.0",
      "Conditions": [
        {
          "condition": "CompareValues",
          "requiredVersion": "1.0.0.0",
          "Value A": { "graphVariable": "RimCombat_ExecutorWeapon", "graphVariableType": "Int" },
          "Comparison": "==",
          "Value B": { "value": 20 }
        },
        {
          "condition": "CompareValues",
          "requiredVersion": "1.0.0.0",
          "Value A": { "graphVariable": "RimCombat_ExecutorWeapon", "graphVariableType": "Int" },
          "Comparison": "==",
          "Value B": { "value": 60 }
        }
      ]
    }
  ]
}
```

### 4. 添加处决者动画（可选）

如果已有武器/种族共享相同的处决者攻击动画，无需额外操作。如果需要独立动画，在 OAR 项目下创建新子集，条件用 `MCO_nextattack` + `RimCombat_ExecutorWeapon`。

---

## 设置伤害倍率

编辑 `Settings.json` → `Execution` → `ExecutionDamageMult`，为新武器类型添加条目。倍率与武器基础攻击力成反比 — 轻武器高倍率，重武器低倍率。参考当前值范围：

| 类别 | 倍率范围 |
|------|:---:|
| Fist | 1.5 - 2.0 |
| Thrust | 2.3 - 3.0 |
| Knock | 1.3 - 2.5 |
| TwoHand | 1.5 - 1.9 |

---

## 调试

在 `Data/SKSE/Plugins/RimCombat/Settings.json` 中开启调试日志后，观察以下关键词：

| 日志关键词 | 含义 |
|------|------|
| `Find executable target` | Find 阶段找到了候选者 |
| `Weapon X Race Y` | 武器|种族组合命中 |
| `Level Execution` | 受害者进入处决硬直（254） |
| `Level Executor` | 处决者进入处决硬直（253） |
| `Execution::ApplyExecutionDamage` | 一次 payload 伤害结算 |

如果没有看到 `Weapon X Race Y` 但受害者已经在 PostureBreak，说明 `Execution/RimCombat.json` 中没有注册该组合。
