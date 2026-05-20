# Rim Combat Developer Notes / TODO

这份文档面向两类人：

1. 继续维护 Rim Combat 本体的开发者。
2. 想基于本项目做动画、规则扩展、联动插件或内容包的模组作者。

它不追求宣传效果，只记录当前代码状态、扩展约定、已知风险和后续待办。

---

## 项目现状审查

### 总体判断

当前项目已经不是“只有概念”的原型，而是已经接通以下主链路的可运行版本：

- 攻击耐力
- 力竭
- 架势
- 格挡 / 限时格挡
- 基础处决
- 战技菜单 / HUD / 分配
- 常规硬直联动与特殊硬直框架

但它仍然明显处于“系统集成阶段”，不是“内容完成阶段”。

当前最强的部分是机制链路。  
当前最弱的部分是边界处理、表现层、NPC 覆盖、完整配置界面和更严格的验证体系。

### 模块完成度

| 模块 | 当前状态 | 说明 |
| --- | --- | --- |
| 攻击耐力 | 已可用 | 普通攻击 / 重击 / payload 耐力消耗已接通 |
| 力竭 | 已可用 | 进入、退出、灰显、攻防倍率联动已接通 |
| 架势 | 已可用 | 架势伤害、恢复、破韧、处决联动已接通 |
| 格挡 / 限时格挡 | 已可用 | 限时窗口、强度倍率、耐力交换、返架势已接通 |
| 战技菜单 / HUD | 已可用 | PrismaUI 菜单、HUD、切换与分配已接通 |
| 战技成长 / 保存 | 半成品 | 等级、点数、解锁已接通，完整收尾仍不足 |
| 战技数据驱动 | 半成品 | payload 路线已经成立，但类型还不够丰富 |
| 处决 | 半成品 | 判定和动画触发已接通，锁定与演出不足 |
| 特殊硬直 | 半成品 | TryStagger / ActionStaggerStart 链路已接通，表现不足 |
| 设置菜单 | 基础可用 | 只有 Load / Reset，真正的模块设置页几乎没做 |
| NPC 战技逻辑 | 未完成 | 玩家侧优先，NPC 尚未真正落地 |
| 多种族处决 | 未完成 | 框架有，实际可用组合很少 |
| 伤害类型 / 抗性 | 占位 | Damage.h 仍是占位接口 |

---

## 当前目录与运行时产物

### 源码结构

- include/Combat：战斗模块头文件
- src/Combat：战斗模块实现
- include/Core、src/Core：Hook、事件、序列化、设置
- include/GUI、src/GUI：PrismaUI / TrueHUD / MenuFramework 相关 UI 层
- dist：当前运行时资源与打包产物

### 当前 dist 产物

- dist/SKSE/Plugins/RimCombat.dll
- dist/SKSE/Plugins/BehaviorDataInjector/RimCombat_BDI.json
- dist/Meshes/Actors/Character/Animations/OpenAnimationReplacer/...
- dist/PrismaUI/...
- dist/RimCombat.7z

### 当前构建方式

- 构建命令：rtk xmake build
- 当前仓库没有项目内自动化测试目标
- 当前最可靠的验证方式仍然是：构建通过 + 游戏内链路测试 + 日志核对

---

## 当前扩展约定

### 动画事件

BehaviorDataInjector 当前注册了以下事件：

| 事件名 | 当前用途 |
| --- | --- |
| RimStamina | 战技或特殊动作的耐力消耗 |
| RimPosture | 架势伤害倍率、是否可破防 |
| RimWeaponArt | 战技状态切换 |
| RimExecution | 处决伤害与处决结束 |
| RimStagger | 特殊硬直等级、免疫、目标修正 |

### Payload 约定

#### RimStamina

- attack|left|倍率
- attack|right|倍率
- attack|auto|倍率
- powerattack|left|倍率
- powerattack|right|倍率
- powerattack|auto|倍率

说明：

- 事件处理时会把 payload 转成小写。
- attackType 目前只有 powerattack 会进入额外倍率逻辑；其他值都会走普通攻击分支。
- side 只接受 left、right、auto。

#### RimPosture

- breakable|true
- breakable|false
- damage|倍率
- end

说明：

- damage|x 用于缓存本次攻击的架势伤害倍率。
- end 当前只会清理 damage cache，不会统一重置 breakable 状态。

#### RimWeaponArt

- start
- end
- prepareend
- toprepare
- cast|spellname

说明：

- 这里负责战技状态切换，以及基于当前战技配置表触发预解析法术特效。
- `cast|spellname` 会在当前战技的 `spells` 配置中查找对应法术。
- spell key 在配置加载时和事件分发时都会统一转成小写后 hash。
- 法术强度使用 `stdMagnitude` 作为 25 级标准强度，再乘常驻/临时技能百分比和技能等级曲线。
- 当前技能倍率公式为：`stdMagnitude * (1 + (SkillMod + SkillPower) / 100) * pow(skillLevel / 25, factor / (factor + 10))`。
- `factor` 当前要求非负。
- `CastSpellImmediate` 实测不会再次应用同一套技能倍率，因此倍率需要在插件侧显式计算。

#### RimExecution

- damage|倍率
- end

#### RimStagger

- immune|等级
- immuneend
- targetset|等级
- targetmodify|偏移
- targetend
- end

说明：

- targetset / targetmodify 的缓存键是攻击者 Actor。
- 真正应用发生在 TryStagger detour 中。
- end 目前只会清理 RimCombat_StaggerLevel，不会清理免疫；免疫要走 immuneend。

### 图变量

| 变量名 | 类型 | 用途 |
| --- | --- | --- |
| RimCombat_WeaponArtID | Int | 当前武器对应战技 ID |
| RimCombat_WeaponArtState | Int | 战技状态，0禁用 / 1准备中 / 2启用 |
| RimCombat_Executable | Bool | 当前是否可处决 |
| RimCombat_ExecutionFlag | Int | OAR 处决组合标志 |
| RimCombat_StaggerLevel | Int | 额外硬直等级 |
| RimCombat_StaggerImmune | Int | 当前硬直免疫等级 |

---

## 当前明确存在或高度可疑的问题

### 1. Posture 的 breakable 缺少统一 reset

当前 RimPosture 的 end 只清理 damageCache，不恢复 RimCombat_PostureBreakable。  
这意味着某个动作只要把 breakable 改成 false，而后续动作没有显式再发 breakable|true，就可能把“不可破防”状态带到下一段动作里。

建议优先级：最高

### 2. NormalAttackConsumeStamina 配置项没有接入行为路径

Settings 里有 NormalAttackConsumeStamina，但当前攻击耐力路径没有使用它。  
结果是：

- 配置可读可写
- 但用户改了以后不会影响实际普通攻击耗耐

这类“配置与行为不一致”的问题比单纯缺功能更难排查。

建议优先级：高

### 3. Execution::TryExecute 的角度判定缺少绝对值

代码注释已经说明 GetHeadingAngle 返回 -180 到 180，绝对值才表示偏角。  
但当前正面判定用的是：

- aggressorHeadingToVictim > 60

而不是对绝对值比较。这样左偏大角度的情况可能会错误通过判定。

建议优先级：高

### 4. Execution 动画发送失败时 flag 清理不完整

TryExecute 在发送动画前会写入 RimCombat_ExecutionFlag。  
如果 NotifyAnimationGraph 失败，当前只会 UnlockActor(victim) 然后返回，但没有在失败路径里统一清理 aggressor / victim 的 execution flag。

结果是 OAR 条件可能保留脏状态。

建议优先级：高

### 5. 处决系统目前实际上只接通了人类目标组合

虽然 Race 枚举和分类已经铺开了很多种族，但当前 availableExcutions 只在构造时插入了 Race::Human 对应组合，GetAnimEvent 也只有 Human 返回可用事件。

因此当前“多种族处决支持”更准确的说法是：

- 识别框架已经铺开
- 实际可用内容目前几乎只有人类目标

建议优先级：中

### 6. 游戏内设置菜单几乎还是空壳

Menu.cpp 里 Stamina/Posture/Block/WeaponArt/Execution 五个渲染函数还是空实现，当前只真正挂了 General 和 Debug。

这意味着：

- 设置菜单在架构上存在
- 但绝大多数设置仍只能改 Settings.json

建议优先级：中

### 7. src/Core/Hooks.cpp 的 clang diagnostic push 没有 pop

这不是运行时 bug，但会把这个文件后续新增的 microsoft-cast 警告一起吞掉，降低静态检查价值。

建议优先级：低

---

## 对外说明时应该坚持的真实表述

后续无论是 README、发布页还是对外说明，建议坚持以下表述，不要再写回旧状态：

- “核心战斗循环已经接通”是对的。
- “处决系统可用，但仍是原型级”是对的。
- “特殊硬直已经有基础链路，不再只是设计概念”是对的。
- “NPC 战技逻辑未完成”是对的。
- “多种族处决目前只是预留框架，不应表述成已支持”是对的。
- “游戏内设置菜单仍不完整，主调参方式仍是 Settings.json”是对的。

---

## 开发优先级 TODO

### P0：应优先修掉的行为问题

1. 为 breakable 增加统一 reset 路径，至少要在 RimPosture end 或状态退出时恢复默认值。
2. 在 Execution::TryExecute 中对角度判定使用绝对值。
3. 在处决动画发送失败路径里统一清理 aggressor / victim 的 RimCombat_ExecutionFlag。
4. 决定 NormalAttackConsumeStamina 这个设置项是要真正接入，还是从设置中删除。

### P1：下一阶段系统完整性

1. 完成 NPC 战技分配与 NPC 使用战技逻辑。
2. 补完整个设置菜单，让 Stamina/Posture/Block/WeaponArt/Execution 页真正可编辑。
3. 继续扩展 payload 类型，把更多战技效果迁移到事件驱动。
4. 明确 breakable、stagger immune、targetmodify 这类规则在动画中的标准使用方式。
5. 给特殊硬直补齐默认动作表现与安全收尾。

### P2：内容与表现层

1. 完善处决系统的位移锁定、旋转锁定、镜头与安全收尾。
2. 补充更多种族、体型和武器组合下的处决支持。
3. 完善限时格挡的特效、音效和反馈。
4. 补齐架势崩溃后的反馈、动画和 AI 反应。
5. 为战技菜单补齐筛选、分类和大列表性能优化。

### P3：开发体验与验证体系

1. 增加一份明确的人工回归清单，覆盖耐力、架势、格挡、处决、战技、特殊硬直。
2. 为关键行为链路补更稳定的日志开关，而不是长期保留大量散日志。
3. 继续收拢与统一模块注释，避免注释还停留在旧行为。
4. 明确 Damage 模块未来是否继续保留；如果短期不用，建议删掉占位接口避免误导。

---

## 建议的人工回归清单

每次改动以下系统之一后，建议至少手动测一轮：

1. 普通攻击、重击、空手攻击的耐力消耗是否符合设置。
2. 0 耐力、力竭进入、力竭退出是否符合预期。
3. 架势伤害、恢复、破韧、处决进入 / 退出是否稳定。
4. 普通格挡与限时格挡的耐力交换、减伤、返架势是否稳定。
5. 战技准备态、启用态、执行中状态与普通攻击边界是否稳定。
6. RimStamina / RimPosture / RimExecution / RimStagger payload 是否按预期工作。
7. TryStagger 覆写后，staggerMagnitude 与 ActionStaggerStart 的等级识别是否一致。
8. 处决动画发送失败或中断时，flag 和角色状态是否能正确回收。

---

## 给基于本项目继续做内容的作者的建议

如果你是基于 Rim Combat 做动画包、战技包或规则扩展，当前最安全的切入方式是：

1. 不要假设所有内容都应该写进 WeaponArt 类字段；优先走 payload。
2. 把普通受击和韧性基础继续交给 Maxsu Poise / Modern Stagger Lock，Rim Combat 侧只处理例外规则和特殊硬直。
3. 使用 RimPosture / RimStagger 时，把开始、结束、复位事件成对写完整，不要只写 start 不写 end。
4. 在还没有统一 reset 机制前，谨慎使用 breakable|false 这类会改变持续状态的 payload。
5. 处决扩展目前最缺的是动画和安全锁定，不是再加更多条件判断。

---

## 当前一句话总结

Rim Combat 现在最像一套“已经接通核心机制、但还在补边界和表现层的战斗框架”。  
继续开发时，优先级应该是修正状态类 bug、收口配置与行为一致性、再扩内容，而不是继续堆新入口。