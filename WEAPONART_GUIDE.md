# Weapon Art Developer Guide / 战技开发指南

**This file is for developers who want to add or edit Weapon Arts for RimCombat.**

**本文档面向希望为 RimCombat 添加或修改战技的 Mod 开发者。**

---

## 1. Beginning / 开始

Weapon Arts are defined by two files:

1. **Configuration JSON**: `Data/SKSE/Plugins/RimCombat/WeaponArt/*.json` — contains the metadata and numerical values.
2. **OAR Animation**: `Meshes/.../OpenAnimationReplacer/YourWeaponArtFolder/WeaponArtName/` — contains animations and frame annotations.

The JSON key name is hashed via `Utils::hash()` into an `int32` ID. Check `RimCombat.log` on startup to see each art's name and ID.

战技由两个文件定义：

1. **配置 JSON**：`Data/SKSE/Plugins/RimCombat/WeaponArt/*.json` — 包含战技元数据和数值。
2. **OAR 动画**：`Meshes/.../OpenAnimationReplacer/你的文件夹/战技名/` — 包含动画和帧注释。

JSON 键名经 `Utils::hash()` 哈希为 `int32` ID。启动后查看 `RimCombat.log` 获取每项战技的名称和 ID。

---

## 2. JSON Configuration / JSON 配置

### Example / 示例

```json
{
    "YourKeyName": {
        "name": "WeaponArtDisplayName",
        "description": "Descriptions",
        "weapons": [],
        "availableWeapon": ["Heavy", "Slash"],
        "consumePoint": 3,
        "unlockLevel": 0,
        "ownAtStart": true,
        "needPrepare": false,
        "stages": {
            "1": {
                "manaCost": 14.0,
                "minMana": 10.0,
                "attacks": {
                    "1": {
                        "left": false,
                        "right": true,
                        "powerAttack": true,
                        "staminaMult": 1.00,
                        "damageMult": 2.00,
                        "poiseDamageMult": 2.50,
                        "postureDamageMult": 2.50,
                        "subStaminaMult": 2.20,
                        "subDamageMult": 0.90,
                        "subPoiseDamageMult": 1.20,
                        "subPostureDamageMult": 1.00
                    }
                }
            }
        },
        "spells": { }
    }
}
```

### Fields / 字段说明

**Format: Field — Type — Required — Description**

| Field | Type | Required | Description |
|-------|------|----------|-------------|
| `name` | string | Yes | Display name in HUD and menus |
| `description` | string | No | Description text in menu. Default: empty |
| `weapons` | string[] | No | Only used when `availableWeapon` contains `Unique`. Format: `["modName.esp\|0xFormID"]` |
| `availableWeapon` | string[] | No | Weapon type flags. Empty or omitted = universal |
| `consumePoint` | int | No | Points required to unlock. Default: 0 |
| `unlockLevel` | int | No | Level required to unlock. Default: 1 |
| `ownAtStart` | bool | No | Granted by default (still costs points to unlock). Default: false |
| `needPrepare` | bool | No | Requires preparation stance (e.g. Iai sheathing). Default: false |
| `stages` | object | No | Stage/Attack numerical config. See below |
| `spells` | object | No | Spell/magic effects config. See Section 3 |

**格式：字段名 — 类型 — 必需 — 说明**

| 字段 | 类型 | 必需 | 说明 |
|------|------|------|------|
| `name` | string | 是 | HUD 和菜单中的显示名称 |
| `description` | string | 否 | 菜单描述文字，默认空 |
| `weapons` | string[] | 否 | 仅 `Unique` 时使用。格式 `["modName.esp\|0xFormID"]` |
| `availableWeapon` | string[] | 否 | 武器类型标志。空或不填 = 通用 |
| `consumePoint` | int | 否 | 解锁消耗点数，默认 0 |
| `unlockLevel` | int | 否 | 解锁所需等级，默认 1 |
| `ownAtStart` | bool | 否 | 初始即拥有（仍需点数解锁），默认 false |
| `needPrepare` | bool | 否 | 是否需要预备态，默认 false |
| `stages` | object | 否 | Stage/Attack 数值配置，见下 |
| `spells` | object | 否 | 法术特效配置，见第三节 |

### Stages & Attacks / 阶段与攻击

All per-hit numerical values are configured in `stages`, **not** in animation payloads. Each stage has a mana cost and a set of attacks:

```json
"stages": {
    "1": {
        "manaCost": 14.0,
        "minMana": 10.0,
        "attacks": {
            "1": { ... },
            "2": { ... }
        }
    },
    "2": {
        "manaCost": 20.0,
        "minMana": 15.0,
        "attacks": { ... }
    }
}
```

**AttackData fields** (all are `float`, default `NaN` = must be set):

| Eligible (sufficient magicka) | Subordinate (insufficient) | Flag |
|------|------|------|
| `staminaMult` | `subStaminaMult` | |
| `damageMult` | `subDamageMult` | |
| `poiseDamageMult` | `subPoiseDamageMult` | |
| `postureDamageMult` | `subPostureDamageMult` | |
| | | `left` (bool) |
| | | `right` (bool) |
| | | `powerAttack` (bool) |

- `manaCost` — magicka consumed when entering this stage (Eligible mode).
- `minMana` — minimum current magicka threshold. If current magicka ≥ this value, Eligible; otherwise Subordinate (no magicka deducted, penalty multipliers applied).
- All 8 float multipliers **must** be set. `NaN` values will cause the art to be skipped on load.

**AttackData 字段**（全部 float，默认 NaN = 必须设置）：

| 魔力充足 (Eligible) | 魔力不足 (Subordinate) | 标志 |
|------|------|------|
| `staminaMult` | `subStaminaMult` | |
| `damageMult` | `subDamageMult` | |
| `poiseDamageMult` | `subPoiseDamageMult` | |
| `postureDamageMult` | `subPostureDamageMult` | |
| | | `left` (bool) |
| | | `right` (bool) |
| | | `powerAttack` (bool) |

- `manaCost` — 进入此阶段消耗的魔力（Eligible 模式）。
- `minMana` — 最低魔力门槛。当前魔力 ≥ 此值时进入 Eligible；否则进入 Subordinate（不扣魔，应用惩罚倍率）。
- 8 个 float 倍率**必须全部设置**。NaN 值会导致该战技被跳过不加载。

### AvailableWeapon / 武器可用值

Weapon types are bitmasks combined via OR. Each value in the list is OR'd together.

武器类型是位掩码，列表中的每个值做 OR 运算。

**1. Weight Class / 重量等级**

| Value | Meaning |
|-------|---------|
| `Light` | Light weapons (Daggers, Claws, Fists) |
| `Normal` | Medium weapons (Swords, Axes, Maces) |
| `Heavy` | Heavy weapons (Greatswords, Battleaxes, Warhammers) |

**2. Attack Type / 攻击类型**

| Value | Meaning |
|-------|---------|
| `Slash` | Slashing (bladed edge) |
| `Thrust` | Thrusting (piercing tip) |
| `Blunt` | Blunt (blunt surface) |
| `Polearm` | Polearms |
| `Range` | Ranged |

**3. Weapon Family / 武器家族**

| Value | Meaning |
|-------|---------|
| `Sword` | Swords |
| `Axe` | Axes |
| `Hammer` | Hammers/Maces |
| `Katana` | Katanas |
| `Spear` | Spears/Lances |
| `Stick` | Staves/Clubs |
| `Whip` | Whips |
| `Fist` | Gauntlets/Fists |
| `Bow` | Bows |
| `Crossbow` | Crossbows |
| `Unique` | Specific weapons (requires `weapons` field) |

**4. Matching Rules / 匹配规则**

- Omitted Weight Class → matches all weights.
- Omitted Weapon Family → matches all families.
- Specified Attack Types → **strict requirement** that must be met.

**5. Examples / 示例**

- `["Slash"]` — all bladed weapons, any weight.
- `["Heavy", "Slash"]` — Heavy + Slashing only (Greatswords/Battleaxes/Polearms).
- `["Thrust"]` — all piercing weapons (Rapiers/Spears/Daggers).

---

## 3. Spells / 法术

A spell is cast via `RimWeaponArt.Cast|SpellName` in an animation frame.

战技可以在动画帧中通过 `RimWeaponArt.Cast|SpellName` 释放法术。

### Example / 示例

```json
"spells": {
    "VacuumChop": {
        "mod": "YourMod.esp",
        "formID": "0xBA21",
        "effectiveness": 1.0,
        "stdMagnitude": 25.0,
        "factor": 10.0,
        "skill": "OneHanded",
        "selfCast": false
    }
}
```

### Fields / 字段说明

| Field | Description |
|-------|-------------|
| `mod` | ESP file name containing the spell |
| `formID` | Spell FormID as hex string |
| `effectiveness` | Spell effectiveness multiplier, typically 1.0 |
| `stdMagnitude` | Standard magnitude. At skill level 25, spell magnitude = this value |
| `factor` | Skill scaling factor. Larger = wider early/late game gap. 0 = no scaling |
| `skill` | Associated skill (OneHanded, TwoHanded, Destruction, etc.) |
| `selfCast` | true = cast on self (for buff-type arts) |

### Formula / 公式

```
ActualMagnitude = stdMagnitude × skillMult × (skillLevel / 25) ^ (factor / (factor + 10))
skillMult = 1 + (SkillModifier + SkillPowerModifier) / 100
```

---

## 4. Animation Payloads / 动画 Payload

In annotation frames, use `EventTag.Payload` format.

在动画的 annotation 帧中，使用 `EventTag.Payload` 格式。

### Weapon Art Lifecycle / 战技生命周期

| Payload | Description |
|---------|-------------|
| `RimWeaponArt.Stage\|N` | Enter stage N. Triggers mana check (from config). Must be on frame 2+ (0.033333s) to avoid BFCO frame 0 double-trigger. |
| `RimWeaponArt.Attack\|stage\|attack` | Execute attack within a stage. Reads all 4 system multipliers from JSON config. One call replaces all old per-system payloads. |
| `RimWeaponArt.End` | Exit weapon art. Clears all cached data for Damage/Posture/Poise/Stagger/Stamina. |
| `RimWeaponArt.ToPrepare` | Transition to Prepare state (only for `needPrepare: true`). |
| `RimWeaponArt.PrepareEnd` | Exit Prepare state → Enable. |
| `RimWeaponArt.Cast\|SpellName` | Cast spell from `spells` config. Only works in Eligible mode. |

| Payload | 说明 |
|---------|------|
| `RimWeaponArt.Stage\|N` | 进入第 N 阶段，触发魔力检查。必须放在第 2 帧或更晚（避开 BFCO 第 0 帧重复触发）。 |
| `RimWeaponArt.Attack\|stage\|attack` | 执行阶段内的一次攻击，从 JSON 配置读取全部 4 系统倍率。一条代替旧版全部子系统 payload。 |
| `RimWeaponArt.End` | 退出战技，清除 Damage/Posture/Poise/Stagger/Stamina 所有缓存。 |
| `RimWeaponArt.ToPrepare` | 切换到预备态（仅 `needPrepare: true` 时使用）。 |
| `RimWeaponArt.PrepareEnd` | 退出预备态 → 启用态。 |
| `RimWeaponArt.Cast\|SpellName` | 释放 spells 配置中的法术。仅 Eligible 模式生效。 |

### Stagger / 硬直

| Payload | Description |
|---------|-------------|
| `RimStagger.TargetSet\|Level` | Attach forced stagger to this attack. Follow with `TargetEnd` after hit. |
| `RimStagger.TargetEnd` | Clear forced stagger state. |

| Payload | 说明 |
|---------|------|
| `RimStagger.TargetSet\|Level` | 附加指定硬直到本次攻击。命中后应接 `TargetEnd`。 |
| `RimStagger.TargetEnd` | 清除附加硬直状态。 |

**Stagger Level IDs:**

| ID | Level |
|----|-------|
| 0 | None |
| 1 | Small |
| 2 | Medium |
| 3 | Large |
| 100 | Knockaway |
| 101 | Knockdown |
| 102 | Strikefly |
| 200 | GuardBreak |

### Block / 格挡

| Payload | Description |
|---------|-------------|
| `RimBlock.GP\|Duration\|Level\|AutoAttack\|NextAttack` | Guard Point window. Duration in ms. Level = stagger on attacker. AutoAttack = auto counter (true/false). NextAttack = combo override (N2/P4/0). |
| `RimBlock.Parry\|Duration` | Parry window. Hit → GuardBreak stagger on attacker. Player-only. |

| Payload | 说明 |
|---------|------|
| `RimBlock.GP\|Duration\|Level\|AutoAttack\|NextAttack` | 精确格挡窗口。Duration=持续时间(ms)。Level=成功GP后给予攻击者的硬直等级。AutoAttack=是否自动反击。NextAttack=反击派生段数(N2/P4/0)。 |
| `RimBlock.Parry\|Duration` | 弹反窗口。受击后给予攻击者 GuardBreak。仅玩家。 |

### Legacy Payloads (Backward Compatible) / 旧版 Payload（向后兼容）

These are still functional for non-WeaponArt animations but should NOT be used in WeaponArt attacks (use `Stage`/`Attack` config instead):

以下在非战技动画中仍可用，但战技攻击中**不应使用**（改用 Stage/Attack 配置）：

| Payload | 说明 |
|---------|------|
| `RimStamina.Consume\|AttackType\|Side\|Multiplier\|FallbackMultiplier` | 旧版耐力消耗 |
| `RimDamage.SetMult\|Multiplier\|FallbackMultiplier` | 旧版伤害倍率 |
| `RimPosture.Damage\|Multiplier\|FallbackMultiplier` | 旧版架势倍率 |
| `RimPoise.TargetSet\|Multiplier\|FallbackMultiplier` | 旧版韧性倍率 |

---

## 5. Payload Template / 帧位模板

### New Config-Driven Template / 新版配置驱动模板

```
0.000000  PIE.@SGVF|MCO_AttackSpeed|1
0.000000  PIE.@SGVI|MCO_nextattack|1
0.000000  PIE.@SGVI|MCO_nextpowerattack|1
0.000000  Collision_AttackStart
0.033333  RimWeaponArt.Stage|1
0.083333  CastOKStart
0.xxx     Collision_Add.node(WEAPON)
0.xxx     weaponSwing
0.xxx     RimWeaponArt.Attack|1|1
0.xxx     RimStagger.TargetSet|Level    ← optional
0.xxx     preHitFrame
0.xxx     HitFrame
0.xxx     Collision_Remove.node(WEAPON)
（after last hit:）
0.xxx     Collision_AttackEnd
0.xxx     RimWeaponArt.End
0.xxx     RimStagger.TargetEnd          ← if TargetSet was used
0.xxx     MCO_WinOpen / MCO_PowerWinOpen
0.xxx     MCO_WinClose / MCO_PowerWinClose
0.xxx     MCO_Recovery
0.xxx     attackStop
```

### Key Rules / 关键规则

1. `RimWeaponArt.Stage` must be at **0.033333s (frame 2) or later** — avoids BFCO frame 0 double-trigger.
2. `Collision_AttackStart` must be at **frame 0** — Precision takes over the entire collision pipeline.
3. `RimWeaponArt.Attack` must match **weaponSwing timestamp** and precede HitFrame.
4. Each swing needs paired `Collision_Add` / `Collision_Remove`.
5. After the final hit, `RimWeaponArt.End` and `Collision_AttackEnd` share the **same frame**.
6. All duration parameters are in **milliseconds**.

1. `RimWeaponArt.Stage` 必须放在 **0.033333（第 2 帧）或更晚**。
2. `Collision_AttackStart` 放**第 0 帧**——整个动画碰撞由 Precision 接管。
3. `RimWeaponArt.Attack` 必须和 **weaponSwing 同帧**，早于 HitFrame。
4. 每段攻击的 Collision_Add/Remove 成对。
5. 最后一段命中后，`RimWeaponArt.End` 和 `Collision_AttackEnd` **同帧**。
6. 所有时间参数单位为**毫秒**。

---

## 6. OAR Configuration / OAR 动画配置

Each art needs a `config.json` in its OAR directory.

每个战技的 OAR 动画目录下需要 `config.json`。

### Example / 示例

```json
{
  "name": "Vacuum Slash",
  "description": "None",
  "priority": 2064141049,
  "conditions": [
    {
      "condition": "AND",
      "requiredVersion": "1.0.0.0",
      "Conditions": [
        {
          "condition": "CompareValues",
          "Value A": {
            "graphVariable": "RimCombat_WeaponArtID",
            "graphVariableType": "Int"
          },
          "Comparison": "==",
          "Value B": {
            "value": 849298816
          }
        },
        {
          "condition": "CompareValues",
          "Value A": {
            "graphVariable": "RimCombat_WeaponArtState",
            "graphVariableType": "Int"
          },
          "Comparison": "==",
          "Value B": {
            "value": 2
          }
        }
      ]
    }
  ]
}
```

### Graph Variables

| Variable | Values |
|----------|--------|
| `RimCombat_WeaponArtID` | Art ID from log |
| `RimCombat_WeaponArtState` | 0=Disabled, 1=Preparing, 2=Enabled |

---

## 7. Notes / 注意事项

1. IDs are hashed from JSON key names via `Utils::hash()`. Check `RimCombat.log` for your art's ID.
2. **Eligible / Subordinate** dual-mode: system auto-selects Eligible or Subordinate multipliers. Design Subordinate values with significant penalty or players will ignore mana costs.
3. Animations are tuned for **BFCO**. MCO users may need manual adjustments.
4. `RimWeaponArt.End` clears **all** caches (Damage/Posture/Poise/Stagger/Stamina) in one call.
5. Multi-hit: send `Attack` payload before **each hit**. Previous cache is auto-overwritten.
6. Unarmed combat has special routing IDs: `Unarm`, `Werewolf`, `Werebear`, `VampireLord`. No `availableWeapon` needed.
7. Without `Collision_AttackStart`, Precision won't process hits. Rim payloads only modify numbers; they don't create collisions.
8. **Testing**: set `consumePoint=0`, `unlockLevel=0`, `ownAtStart=true` for rapid verification.

1. ID 由 JSON 键名经 `Utils::hash()` 哈希得到，查看 `RimCombat.log` 获取。
2. **Eligible / Subordinate 双模式**：系统自动选择倍率。Subordinate 值需有明显的惩罚，否则玩家会忽略魔力。
3. 动画基于 **BFCO** 调校。MCO 用户可能需要自行调整。
4. `RimWeaponArt.End` **一对多清除**所有缓存。
5. 多段攻击：每段命中前发送 `Attack` payload。上一段缓存自动覆盖。
6. 空手路由 ID：`Unarm` / `Werewolf` / `Werebear` / `VampireLord`，无需配置 `availableWeapon`。
7. 无 `Collision_AttackStart` 则 Precision 不处理命中。Rim payload 只修改数值，不创建碰撞。
8. **测试时**：`consumePoint=0`、`unlockLevel=0`、`ownAtStart=true`，方便快速验证。

---

## 8. Complete Example / 完整示例

### JSON

```json
{
    "Impaling Thrust": {
        "name": "贯穿",
        "description": "快速向前突刺，高架势/韧性穿透",
        "weapons": [],
        "availableWeapon": ["Thrust"],
        "consumePoint": 2,
        "unlockLevel": 0,
        "ownAtStart": true,
        "needPrepare": false,
        "stages": {
            "1": {
                "manaCost": 14.0,
                "minMana": 10.0,
                "attacks": {
                    "1": {
                        "left": false,
                        "right": true,
                        "powerAttack": true,
                        "staminaMult": 1.00,
                        "damageMult": 2.00,
                        "poiseDamageMult": 2.50,
                        "postureDamageMult": 2.50,
                        "subStaminaMult": 2.20,
                        "subDamageMult": 0.90,
                        "subPoiseDamageMult": 1.20,
                        "subPostureDamageMult": 1.00
                    }
                }
            }
        }
    }
}
```

### Payload

```
0.000000 PIE.@SGVF|MCO_AttackSpeed|1
0.000000 PIE.@SGVI|MCO_nextattack|1
0.000000 PIE.@SGVI|MCO_nextpowerattack|1
0.000000 Collision_AttackStart
0.033333 RimWeaponArt.Stage|1
0.083333 CastOKStart
0.766667 preHitFrame
0.983333 weaponSwing
0.983333 RimWeaponArt.Attack|1|1
0.983333 Collision_Add.node(WEAPON)
1.083333 HitFrame
1.150000 Collision_Remove.node(WEAPON)
1.150000 Collision_AttackEnd
1.150000 RimWeaponArt.End
1.800000 MCO_WinOpen
1.800000 MCO_PowerWinOpen
2.200000 MCO_WinClose
2.200000 MCO_PowerWinClose
2.200000 MCO_Recovery
2.400000 attackStop
```
