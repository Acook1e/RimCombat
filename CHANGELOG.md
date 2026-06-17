# Rim Combat Changelog

## 0.3.0 — WeaponArt Config-Driven Architecture (2026-06-17)

### WeaponArt 架构重构

- **数据驱动配置系统**：战技数值从动画 payload 完全迁移到 JSON 配置文件。每个战技定义 `stages` → `attacks`，包含 Eligible/Subordinate 两套倍率（`staminaMult`/`damageMult`/`poiseDamageMult`/`postureDamageMult`）。
- **Stage/Attack 双层级**：`RimWeaponArt.Stage|N` 切换阶段（含 manaCost/minMana 魔力检查），`RimWeaponArt.Attack|stage|attack` 触发单次攻击，一次调用同时写入四个子系统。
- **Payload 大幅简化**：每击从旧版 5 条注解（Start + 4 系统）缩减为 2 条（Stage + Attack），Stagger/Cast 注解保留独立通道。
- **`ownAtStart` 拥有权系统**：JSON 中标记 `ownAtStart` 的战技在新游戏/读档时自动授予。战技菜单仅显示已拥有的战技。

### 子系统对齐

- **Stamina**：新增 `Side` 枚举（None/Left/Right/Creature/Auto）和 `RimStaminaConsume` 统一入口，支持 WeaponArt 配置驱动的耐力消耗。
- **Poise/Posture/Damage**：统一新增 `TargetSet(actor, float)` / `SetMult(actor, float)` 重载。变量命名对齐（`poiseMultOnAttack` / `postureMultOnAttack`）。

### 修复

- **多线程崩溃**：`TrackActorUpdate` 的 `lastAttackStates` 从静态局部变量改为类成员，加 `std::mutex` 保护，双重锁作用域修复竞态条件。

### 新增设置

- `bDisablePlayerPostureBreak` — 禁用玩家破防硬直
- `bHideWeaponArtHUDOnSheathe` — 收刀时隐藏战技 HUD
- `fLevelDiffAggressorMultPerLevel` — 等级差削韧衰减因子
- `fVictimAttackingPoiseBonusPercent` — 出手韧性加成比例
- `fStaggerCompensationPercent` — 硬直补偿百分比（加入 MCM 菜单）
- Damage 区域加入 MCM：`UseDamageSystem` / `DamageMultPowerAttack` / `DamageMultBash` / `DamageMultPowerBash`

### 分发与本地化

- **Fomod 安装器**：支持中/英文一键安装。
- **完整双语本地化**：201 个 Localization 键值全量覆盖，MCM 菜单、战技菜单、HUD 均支持中文/英文。
- **Dist 目录重构**：分 `Core`（共用资源）、`Chinese`、`English`、`Fomod` 四层。

### 动画/资源

- 所有 6 个战技的 BFCO 动画已更新为新版 payload 格式。
- 音效/纹理资源重组至 `Core` 目录。

---

## 0.2.0 — Combat Balance Overhaul (2026-06-08)

### 新机制

- **出手韧性（Hyper Armor）**：攻击动画期间（kDraw/kSwing/kHit），受击方获得基于当前武器削韧值的额外韧性。重武器起手时几乎不会被轻攻击打断。
- **等级削韧衰减**：攻击者等级低于受击者时，削韧值按 `1/(1 + 等级差 × 0.05)` 衰减。同级无影响，低级打高级需要更多命中才能打出硬直。

### Bash / Power Bash 定位重构

- **Bash → GP 动作**：无攻击判定。用于触发精确格挡，成功 GP 后自动反击。
- **Power Bash → 弹反**：无攻击判定。成功弹反后给予攻击者 GuardBreak 硬直。
- Bash/Power Bash 的伤害/架势/韧性倍率统一设为 0.50-0.65（兼容旧动画），但耐力消耗惩罚性上涨（1.80-2.00×），不鼓励作为攻击使用。GP 和弹反通过 Rim 注解重写数值。

### 数值调整

- **架势种族基础值**：全种族 ×2.5。Human 40→100，Dragon 88→220。破防所需命中数翻倍，战斗节奏从"压制破防"转为"施压 + GP/弹反制造破绽"。
- **韧性种族基础值**：全种族 ×4。Human 2.5→10，Dragon 7.5→30。韧性池与削韧值重新对齐，避免每刀必触发补偿硬直。
- **双手武器削韧下调**：大剑/战斧/战锤/长柄削韧值降低 15-20%，与新的韧性池匹配。

### 视效/音效

- 添加了限时格挡/GP/弹反的专用视觉和音效反馈，强化操作感。
- 给破防添加了音效。

### 设置新增

- `LevelDiffAggressorMultPerLevel`（0.05）— 等级差削韧衰减因子
- `VictimAttackingPoiseBonusPercent`（0.80）— 出手韧性加成比例

## 0.1.1 — Pre-Overhaul Patch (2026-06-05)

### 修复

- 加入 ModIncomingStager 和 ModTargetStagger 的兼容性
- 加入 ModPowerAttackStaminaCost 的兼容性
- 修复韧性计算包含未装备的护甲

---

## 0.1.0 — Initial Public Test (2026-06-04)

- 首发测试版
- 七个战斗模块：耐力、韧性、架势、硬直、格挡、战技、处决（暂缓）
- 7 个战技：二连斩、贯穿、狩猎巨人、狮子斩、居合横斩/竖劈、真空斩
- 20+ 武器类型、50+ 生物种族的独立数值配置
- PrismaUI 战技菜单与 HUD
- TrueHUD 架势条
- 限时格挡与弹反系统
