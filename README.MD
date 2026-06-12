# Rim Combat · 环战斗

> 面向上古卷轴 5 SE/AE (1.6.629+) 的近战战斗重构。
>
> 当前版本：**0.2.0** — 系统完整，数值经过系统性设计，已进入持续测试与调优阶段。
>
> [Nexus Mods](https://www.nexusmods.com/skyrimspecialedition/mods/180357)

---

## 概述

环战斗不是叠动作模组。它的核心是在 MCO 或 BFCO 等现代动画框架之上，构建一套有博弈感的近战资源系统。

七个独立模块，每个都可以开关与配置：

- **耐力** — 每次挥砍消耗耐力，原版耐力系统被完全替换。25+ 武器类型独立消耗值，50+ 生物种族独立配置。附带力竭系统和零耐力禁攻机制。
- **韧性（Poise）** — 内部硬直阈值。轻击削韧、重击破韧。轻/重甲按部位独立提供韧性加成。出手韧性机制使重武器起手不易被打断。等级差削韧衰减防止低级角色无限压高级敌人。
- **架势（Posture）** — 破防压力资源。架势归零触发失衡硬直并升级当次攻击的硬直等级。通过 TrueHUD 可视化。
- **硬直（Stagger）** — 基于 Modern Stagger Lock 的四级硬直系统。特殊硬直级别包括击飞、砸趴、挑飞、破防崩溃。
- **格挡** — 每把武器独立格挡强度。150ms 限时格挡窗口。Bash 转为 GP 动作，Power Bash 转为弹反——两者均无攻击判定。
- **战技** — 7 个首发战技，消耗魔力释放。Eligible/Subordinate 双模式。PrismaUI 菜单分配，经验成长与点数解锁。
- **处决** — 三种基础人型处决动画（空手/单手/双手）开发中，JSON 注册表支持外源扩展。

---

## 数值系统

全部数值通过 `Settings.json` 配置：

- 四种攻击类型 × 四个维度（伤害 / 架势 / 韧性 / 耐力）的全局倍率
- 25+ 武器类型的独立基础值（耐力消耗、架势伤害、韧性伤害、格挡强度）
- 50+ 生物种族的独立基础值
- 护甲部位韧性加成、硬直阈值、恢复时间、再生率

战技的伤害 / 架势 / 韧性 / 耐力由动画 Payload 独立控制，不依赖全局设置。

---

## 模组开发者接口

环战斗提供一套 Payload 注释系统：

- `RimStamina` — 耐力管线（Start / End / Consume）
- `RimDamage` — 伤害管线（SetMult / End）
- `RimPosture` — 架势管线（Damage / End / Unbreakable）
- `RimPoise` — 韧性管线（TargetSet / TargetEnd / Set / End）
- `RimStagger` — 硬直线（TargetSet / TargetEnd / Immune）
- `RimWeaponArt` — 战技管线（Start / End / PrepareEnd / ToPrepare / Cast）
- `RimBlock` — 格挡管线（GP / Parry）
- `RimExecution` — 处决管线（Damage / SetDistance / End）

所有 Payload 支持 Eligible / Subordinate 双模式自动切换。

详见 [WEAPONART_GUIDE.md](WEAPONART_GUIDE.md)。

---

## 前置

**必须：**
- SKSE 1.6.629+
- Address Library
- OAR（Open Animation Replacer）
- MCO 或 BFCO

**推荐：**
- PrismaUI — 战技菜单与 HUD
- TrueHUD — 架势条
- Modern Stagger Lock — 硬直动画兼容
- OCF — 更细武器分类
- Precision — 精确碰撞检测
- Behavior Data Injector — 战技动画注入

---

## 文件结构

```
dist/
├── SKSE/Plugins/RimCombat/
│   ├── RimCombat.dll
│   ├── Settings.json          ← 运行时配置
│   ├── SettingsDefault.json   ← 默认配置模板
│   ├── Localization.json      ← 中文菜单文本
│   ├── WeaponArt/             ← 战技 JSON 配置
│   ├── Execution/             ← 处决注册表
│   ├── Poise/                 ← 特殊韧性覆盖
│   ├── Posture/               ← 特殊架势覆盖
│   └── Stagger/               ← 弹射物硬直映射
├── EN/SKSE/Plugins/RimCombat/
│   └── Localization.json      ← 英文菜单文本
└── Meshes/.../RimCombat - WeaponArt/  ← 战技 OAR 动画
```

---

## 已知问题

- NPC 暂不能使用战技
- 处决动画开发中
- 平衡性处于早期迭代阶段，数值可能继续调整

---

## 版本历史

| 版本 | 日期 | 内容 |
|------|------|------|
| 0.2.0 | 06-08 | 架势/韧性池大幅上调，Bash→GP / Power Bash→弹反重构，出手韧性，等级削韧衰减，Perk Entry 重新接入 |
| 0.1.0 | 06-04 | 首发测试版，七个模块 + 七个战技 |

详见 [CHANGELOG.md](CHANGELOG.md)。

---

## 致谢

CommonLibSSE-NG · MCO · BFCO · OAR · Maxsu Poise · Modern Stagger Lock · Elden Rim
