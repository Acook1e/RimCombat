# Rim Combat · 环战斗

> Skyrim SE/AE (1.6.629+) 近战战斗重构。
>
> 当前版本：**0.4.0** — 处决系统完整实现，7 个模块。
>
> [Nexus Mods](https://www.nexusmods.com/skyrimspecialedition/mods/180357) · [更新日志](CHANGELOG.md) · [开发者文档](doc/) · [战技自定义指南](EXECUTION_GUIDE.md)

---

## 概述

Rim Combat 在 MCO / BFCO 等现代动画框架之上构建一套**资源驱动的近战博弈系统**。它不是动作模组——不添加新攻击动画，而是重建攻击命中后的全部数值管线。

**七个模块**，每个可独立开关，全部参数通过 `Settings.json` 配置：

| 模块 | 功能 |
|------|------|
| **耐力** | 攻击消耗、零耐力禁攻、力竭系统。20+ 武器类型 × 50+ 生物种族独立基础值 |
| **韧性** | 内部硬直阈值。轻击削韧、重击破韧。四部位护甲独立加成。出手韧性、等级衰减 |
| **架势** | 破防压力资源。架势归零触发失衡硬直。TrueHUD 可视化 |
| **硬直** | 四级常规 + 七种特殊硬直。同等级补偿加速脱离 |
| **格挡** | 每武器独立格挡强度。限时格挡 150ms。Bash→GP / Power Bash→弹反 |
| **战技** | 7 个首发战技。JSON 配置驱动。魔力阈值双模式。PrismaUI 菜单 |
| **处决** | JSON 注册表驱动。四种动画类别。Find→Execute 原子管线。延迟杀死。处决无敌 |

---

## 处决系统

当受害者的架势被击破（PostureBreaks），进入可处决状态。处决者按攻击键（右键/ActionRightAttack）触发：

1. **FindExecutableTarget** — 250 单位距离内、前方 60° 锥、高度差 ≤ 50、武器|种族组合可用 → 原子认领受害者
2. **Execute** — 处决者麻醉、图变量设置、150 单位定位、朝向对齐、`attackStart` 硬直
3. **动画** — OAR 根据 `ExecutorWeapon` + `VictimRace` + `MCO_nextattack` 选择对应处决动画
4. **伤害** — 动画 payload（`RimExecution → damage|Mult`）逐段结算，最后一段留 0.5HP，`ExecutionEnd` 完成杀死

四种动画类别：**Thrust**（单手刃）、**Knock**（钝器/重击）、**TwoHand**（双手武器）、**Fist**（空手/爪/拳套）。

自定义处决组合：编辑 `Execution/RimCombat.json`（种族→武器→方向），详见 [EXECUTION_GUIDE.md](EXECUTION_GUIDE.md)。

---

## 数值系统

所有数值通过 `SKSE/Plugins/RimCombat/Settings.json` 集中配置：

- 四种攻击类型 × 四个维度（伤害 / 架势 / 韧性 / 耐力）的全局倍率
- 25+ 武器类型的独立基础值
- 50+ 生物种族的独立基础值
- 护甲部位韧性、硬直阈值、恢复时间、再生率
- 种族/NPC 韧性覆盖（`Poise/*.json`）、架势覆盖（`Posture/*.json`）
- 弹射物硬直映射（`Stagger/*.json`）
- 处决伤害倍率（`ExecutionDamageMult`）— 23 种武器类型独立倍率

战技的伤害/架势/韧性/耐力由 `WeaponArt/*.json` 中的 `stages→attacks` 层级定义，与全局设置叠加计算。详见 [WEAPONART_GUIDE.md](WEAPONART_GUIDE.md)。

---

## 模组开发者接口

在动画 annotation 帧中添加以下图事件，由 Rim Combat 接管数值管线：

| 事件 | Payload | 说明 |
|------|---------|------|
| `RimStamina` | `Start` / `End` / `Consume\|AttackType\|Side\|Mult` | 耐力接管与消耗 |
| `RimDamage` | `SetMult\|Mult` / `End` | 基础伤害倍率 |
| `RimPosture` | `TargetSet\|Mult` / `TargetEnd` / `Unbreakable\|Duration` | 架势伤害与免疫 |
| `RimPoise` | `TargetSet\|Mult` / `TargetEnd` / `Set\|Mult` / `End` | 韧性伤害攻防双方 |
| `RimStagger` | `TargetSet\|Level` / `TargetEnd` / `Immune\|Duration` | 硬直附加与免疫 |
| `RimWeaponArt` | `Stage\|N` / `Attack\|N\|M` / `End` / `PrepareEnd` / `ToPrepare` / `Cast\|Name` | 战技状态机与法术 |
| `RimBlock` | `GP\|Duration\|Level\|Auto\|Next` / `Parry\|Duration` | 精确格挡与弹反 |
| `RimExecution` | `Damage\|Mult` / `End` | 处决伤害结算 |

> Duration 单位均为毫秒。Stagger Level 定义：0=None, 1=Small, 2=Medium, 3=Large, 4=Largest, 100=Knockaway, 101=Knockdown, 102=Strikefly, 200=GuardBreak, 253=Executor, 254=Execution, 255=PostureBreak。

---

## 前置

**必须：**

- SKSE 1.6.629+
- Address Library
- OAR (Open Animation Replacer)
- MCO 或 BFCO

**推荐：**

- PrismaUI — 战技菜单与 HUD
- TrueHUD — 架势条 + 力竭灰屏
- Modern Stagger Lock — 硬直动画
- OCF — 武器细分类
- Precision — 精确碰撞
- Behavior Data Injector — 动画注入

---

## 文件结构

```
dist/
├── Core/                              ← 共用资源
│   ├── SKSE/Plugins/RimCombat/
│   │   ├── RimCombat.dll
│   │   ├── Settings.json
│   │   ├── SettingsDefault.json
│   │   ├── Localization.json
│   │   ├── WeaponArt/*.json
│   │   ├── Execution/*.json
│   │   ├── Poise/*.json
│   │   ├── Posture/*.json
│   │   └── Stagger/*.json
│   ├── Meshes/RimCombat/              ← 火花粒子特效
│   ├── Sound/FX/RimCombat/            ← Parry/GP/破防/处决音效
│   └── RimCombat.esp                  ← 空插件（加载锚点）
├── CN/SKSE/Plugins/RimCombat/
│   └── Localization.json              ← 中文文本
├── EN/SKSE/Plugins/RimCombat/
│   └── Localization.json              ← 英文文本
├── Fomod/
│   ├── info.xml
│   └── ModuleConfig.xml
├── Meshes/.../RimCombat - Stagger/    ← 处决受害者 OAR 动画
└── Meshes/.../RimCombat - WeaponArt/  ← 战技 OAR 动画
```

---

## 构建

```bash
# 克隆
git clone --recurse-submodules https://github.com/Acook1e/RimCombat.git

# 构建（需要 xmake + Visual Studio 2022）
xmake build RimCombat
```

GitHub Actions 自动构建输出在 [Releases](https://github.com/Acook1e/RimCombat/releases)。

---

## 已知问题

- NPC 暂不能使用战技
- 处决仅支持人型种族

---

## 致谢

[CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) · SKSE · MCO · BFCO · OAR · Modern Stagger Lock · Elden Rim · Maxsu Poise · PrismaUI · TrueHUD · OCF · Achang · black · Dtry · Maxsu · 旧日火 · 回声
- 平衡性持续迭代

---

## 致谢

[CommonLibSSE-NG](https://github.com/CharmedBaryon/CommonLibSSE-NG) · SKSE · MCO · BFCO · OAR · Modern Stagger Lock · Elden Rim · Maxsu Poise · PrismaUI · TrueHUD · OCF · Achang · black · Dtry · Maxsu · 旧日火 · 回声
