# Rim Combat · 环战斗 — 完整更新日志

> 基于 git 提交历史 (2025-12-21 ~ 2026-06-26)，7 个版本。

---

## 0.4.0 — 处决系统完整实现 (2026-06-26)

### 处决系统

- **Find → Execute 管道**：`FindExecutableTarget` 在锁内完成全部验证（距离 250、高度差 50、前方 60° 锥、武器|种族组合存在、方向可用、原子 erase），返回 `(victim, direction)` 元组。`Execute` 无条件保证成功，含 `attackStart` 失败重试。
- **四种处决动画类别**：Thrust（匕首/剑/细剑/武士刀/枪）、Knock（单手斧/锤/战斧/战锤）、TwoHand（大剑/大太刀/长柄刀/长枪/斧枪）、Fist（空手/法术/爪/拳套）。正向/背刺由 `Execution.json` + OAR 动画方向区分。
- **OAR 受害者端配置**：四个 `config.json`（`RimCombat - Stagger/Execution - Human - *`），通过 `RimCombat_StaggerLevel == 254` + `ExecutorWeapon` + `VictimRace` 条件匹配。MSL 动画 Forward/Backward 变体自动选择。
- **处决伤害管线**：`baseDamage × ExecutionDamageMult × payload`。23 种武器类型独立倍率（Settings.json → `ExecutionDamageMult`）。动画 payload（`RimExecution → damage|Mult`）控制每段分配。
- **延迟杀死机制**：每次攻击 payload 锁定受害者血量 ≥ 0.5HP。`ExecutionEnd` 检测血量 < 1HP 执行最终杀死。
- **处决无敌**：IsGhost detour 覆盖受害者侧（外部弹射物/近战命中穿透）。处决者侧走 GodMode 或伤害 hook 过滤。
- **AI 锁**：处决者 `boolBits.set(true, kParalyzed)`（保留行为图处理，仅锁移动/输入）。
- **新硬直等级**：`Executor = 253`（处决者动画）、`Execution = 254`（受害者动画），与 `PostureBreak = 255` 组成处决三级链路。
- **处决 JSON 注册表**：`Execution/RimCombat.json`。Race-first 结构（`"Human": {"Sword": ["Front","Back"], ...}`）。19 种武器覆盖。
- **攻击输入钩子**：`ActionRightAttack` 中 `FindExecutableTarget → Execute`，返回 false 拦截原攻击动作，直接进入处决。
- **处决音效**：轻击/重击独立 WAV（`RimCombat.esp` FormID 0x809/0x80A）。

### 战技

- **JSON Schema**：`scripts/WeaponArt.schema.json` + VS Code 本地映射（`.vscode/settings.json`）。编辑器内自动校验 + 补全。
- **WeaponArtHUD**：重构 `canShow` 为 `drawn & !menuConflict` 三态分离。`ControlMap::contextPriorityStack` 替 14 次 `IsMenuOpen` 轮询。`kMap` 上下文特殊检测。

### 修复

- **硬直无限循环**：`Stagger::Update` 中 `continue` 未推进迭代器，PostureBreak 专用等级触发死循环。
- **硬直升级重入**：`staggerStop → staggerStart` 同步重入导致卡死，改为 `SetLevel(None)` 先于事件发送。
- **HUD 闪烁**：`CanShow()` 内 `!isShow` 导致逐帧 Show/Hide 交替。
- **MenuEvent 条件反向**：阻塞菜单判定布尔值反转，HUD 在背包界面显示。
- **`string_view` 悬垂**：`menuOpen` 从 `string_view` 改为 `uint32_t hash`。

---

## 0.3.0 — 数据驱动战技架构 (2026-06-17)

**提交：** `5c899ee` `9830b92` `43d8ae5` `2bff272`

### WeaponArt 架构重构

- **JSON 配置驱动**：战技的伤害、架势伤害、韧性伤害、耐力消耗全部从 `WeaponArt/*.json` 配置文件读取，不再写在动画 payload 里。每个战技定义 `stages` → `attacks` 层级结构。
- **Eligible / Subordinate 双模式**：进入 Stage 时检查魔力。魔力 ≥ `minMana` → Eligible（扣魔力，用主倍率），魔力 < `minMana` → Subordinate（不扣魔力，用降级倍率）。两套独立的 8 个浮点乘数（`staminaMult` / `damageMult` / `poiseDamageMult` / `postureDamageMult` × Eligible/Subordinate）。
- **Payload 大幅精简**：每击从旧版 5 条注解削减为 2 条（`Stage|N` + `Attack|N|M`）。`Attack` 事件一次性写入四个子系统。
- **魔法值检查**：`Stage` 事件携带 `manaCost` 和 `minMana`。Subordinate 模式下仅取消伤害增益，不阻断动画播放。
- **Cast 法术系统**：法术强度 = `stdMagnitude × skillMult × (skillLevel/25)^(factor/(factor+10))`。支持绑定任意技能等级、缩放因子、自身施法。`Cast` 事件调用 `CastSpellImmediate`。
- **`ownAtStart` 拥有权**：标记为 `ownAtStart` 的战技在新游戏/读档时自动授予玩家。菜单仅显示已拥有的战技。
- **`needPrepare` 准备动画**：`ToPrepare` → 准备状态 → `PrepareEnd` → 启用状态。支持战技的"拔刀"类前置动画。
- **6 个战技动画全部更新**为 0.3.0 payload 格式：二连斩、贯穿、狩猎巨人、狮子斩、居合横斩/竖劈、真空斩。
- **7 个首发战技 JSON 配置**全部重写：`DoubleSlash.json`、`ImpalingThrust.json`、`GiantHunt.json`、`LionsClaw.json`、`HorizontalSlash.json`、`VerticalSlash.json`、`VacuumSlash.json`。

### 子系统对齐

- **Stamina**：新增 `Side` 枚举（None/Left/Right/Creature/Auto）。新增 `RimStaminaConsume(actor, side, powerAttack, multiplier)` 统一入口，支持 WeaponArt 配置驱动耐力消耗。`PayloadParse` 移除 `Consume` 分支（合并入 `Attack`）。
- **Poise**：新增 `Set(actor, float)` / `TargetSet(actor, float)` 重载。`poiseMultOnAttack` 和 `poiseMultOnHit` 分别缓存攻击方和被击方倍率。移除 `PayloadParse` 中的 `Set`/`End` 分支。
- **Posture**：新增 `TargetSet(actor, float)` 重载。`postureMultOnAttack` 缓存攻击方倍率。移除 `PayloadParse` 中的 `TargetSet`/`TargetEnd` 分支。
- **Damage**：新增 `SetMult(actor, float)` 重载。移除 `PayloadParse` 中的 `SetMult`/`End` 分支。
- **Execute**：`EnterExecutable` / `ExitExecutable` / `TryExecute` / `FindExecutableTarget` 函数补全。`ExecuteDamageMult` payload 通道。
- **Stagger**：WeaponArt Subordinate 状态绕过 `TargetSet`/`TargetEnd` payload 分支。

### 修复

- **多线程崩溃**：`TrackActorUpdate` 的 `lastAttackStates` 从无锁 `static` 局部变量改为带 `std::mutex` 保护的静态类成员，修复竞态条件。
- **LoadSettings 空指针**：`Settings::LoadSettings()` 在首次调用时 DataHandler 未初始化导致崩溃，加入空指针守卫和延迟加载。
- **Stagger Recovery 无限循环**：同等级硬直多次触发导致恢复计时异常，改为重置恢复且不重复播放动画，大硬直降级逻辑修正。
- **版本号错误**：`Fomod/info.xml` 版本号从旧版号更新为 0.3.0。
- **Poise 默认值修正**：`fStaggerCompensationPercent` 和 `fVictimAttackingPoiseBonusPercent` 名称一致性问题修正。

### 分发

- **Fomod 安装器**：新建 `Fomod/ModuleConfig.xml` + `Fomod/info.xml`，支持中/英文一键安装。dist 目录重构为 `Core`（共用资源）、`CN`（中文）、`EN`（英文）、`Fomod` 四层。
- **完整双语本地化**：201 个 Localization 键值全量覆盖（`CN/` 中文、`EN/` 英文）。MCM 菜单、战技菜单、HUD 均中英双语文。
- **默认配置文件**：新增 `SettingsDefault.json`，`Settings.json` 由安装器从模板生成。
- **动画/音效资源重组**：Parry/GP/PostureBreak 音效和火花粒子特效移至 `Core/` 共用目录。OAR 动画结构重组，按功能分目录（Parry/Stagger/Execution/WeaponArt）。

### 新增设置

```
bDisablePlayerPostureBreak        — 禁用玩家破防硬直
bHideWeaponArtHUDOnSheathe        — 收刀时隐藏战技 HUD
fLevelDiffAggressorMultPerLevel   — 等级差削韧衰减因子
fVictimAttackingPoiseBonusPercent — 出手韧性加成比例
fStaggerCompensationPercent       — 硬直补偿百分比
bTimedBlockNeverPostureBreak      — 限时格挡免疫架势崩溃
fWeaponArtMenuStartPercent        — 战技菜单起始位置
```

### 新增子系统配置文件

- `Poise/Skyrim.json` — 种族 + NPC 基础韧性覆盖
- `Posture/Skyrim.json` — 种族 + NPC 基础架势覆盖
- `Stagger/EldenRim.json` — 弹射物硬直等级映射缓存

### 代码品质

- **include 重整化**：移除 6 处冗余头文件包含，19 处源文件冗余传递依赖，清理未使用的第三方库引用。
- **WeaponArt Guide**：编写完整的 13KB 中文战技自定义指南（`WEAPONART_GUIDE.md`），包含 JSON 结构、武器类型标志、Stage/Attack 体系、法术公式推导和完整模板。

---

## 0.2.0 — 战斗平衡大修 (2026-06-08)

**提交：** `4dc2fce` `6c0001b`

### 新机制

- **出手韧性（Hyper Armor）**：攻击动画期间（`kDraw`/`kSwing`/`kHit`），受击方获得基于当前武器削韧值的额外韧性。重武器起手时几乎不会被轻攻击打断。倍率由 `fVictimAttackingPoiseBonusPercent` 控制（默认 0.80）。
- **等级削韧衰减**：攻击者等级低于受击者时，削韧值按 `1/(1 + 等级差 × fLevelDiffAggressorMultPerLevel)` 衰减（默认因子 0.05）。同级无影响。低级打高级需要更多命中才能打出硬直。

### Bash / Power Bash 定位重构

- **Bash → GP 动作**：无攻击判定。用于触发精确格挡，成功 GP 后自动反击。反击段数可配置（`N2`/`P4`/`0`）。
- **Power Bash → 弹反**：无攻击判定。成功弹反后给予攻击者 `GuardBreak` 硬直（等级 200）。
- Bash/Power Bash 的伤害/架势/韧性倍率统一设为 0.50-0.65（兼容旧动画），但耐力消耗惩罚性上涨（1.80-2.00×），不鼓励作为攻击使用。

### 数值调整

- **架势种族基础值**：全种族 ×2.5。Human 40→100，Dragon 88→220，Giant 75→188。破防所需命中数翻倍，战斗节奏从"压制破防"转为"施压 + GP/弹反制造破绽"。
- **韧性种族基础值**：全种族 ×4。Human 2.5→10，Dragon 7.5→30。韧性池与削韧值重新对齐，避免每刀必触发补偿硬直。
- **双手武器削韧下调**：大剑/战斧/战锤/长柄削韧值降低 15-20%，与新的韧性池匹配。
- **Perk Entry Point 兼容**：重新接入 `ModTargetStagger`（攻击方 Perk 硬直加成）和 `ModIncomingStagger`（受击方 Perk 硬直减免），应用 0.6-2.0 钳制。

### 设置新增

```
fLevelDiffAggressorMultPerLevel   0.05  — 每级等级差的削韧衰减因子
fVictimAttackingPoiseBonusPercent 0.80  — 出手韧性占武器削韧值的比例
fStaggerCompensationPercent       0.33  — 连续硬直补偿比例
```

---

## 0.1.2 — 硬直恢复修复 + 本地化重构 (2026-06-12)

**提交：** `a02a08f`

### 修复

- **Stagger Recovery 重写**：同等级硬直重复触发时的恢复逻辑完全重构。不再无限叠加恢复计时，改为重置恢复时间并加速此次恢复。大硬直会降级为次一等级的硬直而非直接清除。
- **硬直补偿系统**：`fStaggerCompensationPercent` 计算逻辑修正。

### 本地化

- 中/英双语 Localization 完全分拆（`CN/` 和 `EN/` 目录），201 个键值对全量覆盖。
- `EldenRim.json` 中战技名称和描述全部完成双语适配。
- MCM 菜单中文文本完全本地化。

---

## 0.1.1 — Poise 默认值修正 (2026-06-08)

**提交：** `6c0001b`

### 修复

- `fVictimAttackingPoiseBonusPercent` 和 `fStaggerCompensationPercent` 命名修正，与设置名称对齐。
- `Settings.json` 和 `SettingsDefault.json` 中相关字段同步更新。
- `Poise::DamagePoiseHealth` 中的补偿计算使用正确的设置字段。

---

## 0.1.0 — 首发测试版 (2026-06-04 ~ 2026-06-04)

**提交：** `e07b0a6` `a95c1a7`

### 七个战斗模块

- **耐力（Stamina）**：完全接管原版系统。轻/重击/盾击/强力盾击独立消耗。20+ 武器类型独立基础消耗。50+ 生物种族独立配置。零耐力禁攻。力竭系统 + TrueHUD 灰屏反馈。
- **韧性（Poise）**：内部硬直阈值系统。轻甲/重甲四部位独立加成。`ModTargetStagger` / `ModIncomingStagger` 兼容。种族/NPC 覆盖配置。
- **架势（Posture）**：破防资源系统。最大架势 = 生命值 × 倍率 + 耐力值 × 倍率 + 种族基础值。归零触发 PostureBreak 硬直并升级当次攻击。TrueHUD 特殊栏可视化。
- **硬直（Stagger）**：基于 MSL 的四级硬直（Small/Medium/Large/Largest） + 五种特殊硬直（Knockaway/Knockdown/Strikefly/GuardBreak/PostureBreak/Execution）。免疫缓存。恢复机制。
- **格挡（Block）**：每把武器独立格挡强度。限时格挡 150ms 窗口。Bash→GP / Power Bash→弹反（重构在 0.2.0）。弹射物反弹。
- **战技（WeaponArt）**：7 个首发战技。PrismaUI 菜单 + HUD。经验与点数解锁。武器类型匹配。Payload 驱动数值。
- **处决（Execution）**：三种基础人型处决（空手/单手/双手）。JSON 注册表。Executable 缓存。多处决触发路径。

### 数值系统

- 耐力消耗完整重做：`Stamina::SwingStaminaConsume` / `CreatureStaminaConsume` / `BashStaminaConsume` / `UnarmStaminaConsume` 四条路径。每种武器类型独立 `baseStaminaCostMap`。
- 伤害归一化：`Damage::ProcessWeaponDamage` 按原版基准反推基础伤害，再乘 RimCombat 倍率，确保 BFCO/MCO 的不同攻击倍率归一化。
- `Settings.json` 包含 80+ 设置项，覆盖耐力、韧性、架势、硬直、格挡、战技六大区域。
- `SettingsDefault.json` 作为默认配置模板。

### 模组支持

- OCF 武器分类：支持 25+ 武器子类型（匕首/爪/剑/细剑/武士刀/单手斧/锤/拳套/鞭/枪/大剑/大太刀/战斧/战锤/长棍/长柄刀/长枪/斧枪/弓/弩/法杖/盾牌/火把）。
- 50+ 生物种族通过行为图路径映射识别。
- MSL 硬直动画兼容。
- Parry/GP 视觉特效和音效（火花粒子 + 专用 WAV）。

### 已知限制

- NPC 暂不能使用战技。
- 处决系统开发中，动画不完整。

---

## Pre-0.1.0 — 开发阶段 (2025-12-21 ~ 2026-06-03)

**提交：** `ba9c358` ~ `2b41af7`（38 次提交）

### 2025-12-21 ~ 2025-12-24：项目初始化

- 仓库创建，CommonLibSSE-NG 子模块，xmake 构建系统。
- TrueHUD API 集成，SKSE Menu Framework 集成。
- 基础钩子框架：`Hook_OnMainUpdate`、`Hook_OnActorUpdate`。

### 2026-01-08：处决系统 WIP

- `ExecutionHandler` 骨架搭建，可处决 Actor 缓存。

### 2026-02-07 ~ 2026-02-08：重构 + CI

- 项目结构重构，命名空间迁移。
- `ModActorValue` 钩子加入，`Stamina` 和 `Posture` 模块拆分。
- GitHub Actions 自动构建 CI 搭建。

### 2026-02-08 ~ 2026-02-10：耐力 + 格挡 + 菜单

- 攻击耐力系统实现（轻击/重击/盾击/生物攻击）。
- 限时格挡系统实现（窗口检测 + 倍率加成）。
- MCM 菜单框架（ImGui）和本地化系统。

### 2026-02-17：战技系统 WIP

- `WeaponArt` 核心数据结构：`WeaponArtInfo`、`Manager`、`PlayerStat`。
- 武器类型匹配位掩码系统（`AvailableWeapon`）。
- JSON 配置加载器。

### 2026-04-30 ~ 2026-05-04：大规模重构

- 项目完全重组为 `Combat/`、`Core/`、`Data/`、`GUI/` 模块。
- PrismaUI API 集成，战技菜单 HTML/CSS/JS 前端。
- 战技菜单 + HUD 完整实现。
- 本地化框架（`Localization.h`/`.cpp` + `Localization.json`）。
- 序列化系统（`Serialization.h`/`.cpp` + 各模块 `serialType` 注册）。

### 2026-05-10 ~ 2026-05-24：战斗系统逐项完善

- 战技触发更新（`SwitchWeaponArt`、`Stage`、`Attack`、`End`、`Cast`）。
- 处决系统迭代：`TryExecute`、`FindExecutableTarget`、碰撞禁用、距离设置。
- 攻击禁用系统：零耐力禁攻（玩家/NPC），图事件后门。
- 武器类型系统完成：OCF 关键词识别 + 原版 `WEAPON_TYPE` 回退。
- 硬直处理系统：四级硬直 + 特殊硬直等级枚举，弹射物硬直映射缓存。
- 伤害系统：`SetMult`/`End` 缓存管道。
- 战技法术释放（`CastSpellImmediate`）。
- 韧性系统完整实现：`PoiseData`、`CalculateMaxPoise`、`ProcessWeaponHit`、`DamagePoiseHealth`。
- 生物支持完成：50+ 种族行为图映射，生物耐力/韧性/架势基础值。
- 弹反和 GP 系统完整实现：`GPData`、`ParryEndTimes`、自动反击。

### 2026-06-02 ~ 2026-06-03：API 集成 + 耐力重做

- InputManager API 集成：Alt/T 按键注册，按住/按下双模式。
- 耐力消耗系统完全重写：统一 `RimStamina` 事件管道，`Consume|AttackType|Side|Multiplier` payload 格式。
- 主循环优化：`MainUpdate` 中各模块 `Update` 函数按需调用，仅在系统启用时执行。

---

## 统计

| 版本 | 日期 | 提交数 | 主要变更 |
|------|------|:---:|------|
| Pre-0.1.0 | 2025-12-21 ~ 2026-06-03 | 38 | 项目初始化、七个模块骨架、CI、重构 |
| 0.1.0 | 2026-06-04 | 2 | 首发测试版，耐力重做，MainUpdate 修复 |
| 0.1.1 | 2026-06-08 | 1 | Poise 默认值修正 |
| 0.1.2 | 2026-06-12 | 1 | 硬直恢复重写，中英双语文拆分 |
| 0.2.0 | 2026-06-08 | 2 | 战斗平衡大修，出手韧性，等级衰减，Bash→GP |
| 0.3.0 | 2026-06-17 | 4 | 数据驱动战技架构，Fomod，include 重整化 |

**总计：52 次提交，7 个版本，8 个月开发周期。**
