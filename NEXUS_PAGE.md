# Rim Combat

环战斗

中文版本

环战斗是一次对原版天际战斗策略和深度的重整。它的目的不是为了更花哨的动作，而是在 MCO 或 BFCO 这类现代战斗框架下——战斗却仍然只有数值碾压或满地打滚——提供一套真正有博弈感的近战系统。

环战斗通过 OCF 分类系统，支持来自模组和原版的 20 多种武器和 50 多种生物的详细数值划分。

环战斗包含七个模块，每个都可以独立开关与配置：

---

耐力模块

我重新定义了攻击的耐力消耗。启用后，原版的攻击耐力消耗会被完全取消，你不用担心原版不合理的天价重击耐力了。

每一次轻攻击都会消耗耐力，重击、盾击（bash）、格挡猛击（power bash）亦然。每种武器类型——包括模组新增的武器子类型——都有独立的基础消耗值。生物的攻击同样纳入这套体系，不同种族的爪击、撕咬、撞击各自读取各自的耐力配置。

包含一个极为强大的攻击禁用机制：任何角色耐力为 0 的状态都无法攻击，防止无限连招。但攻击直接通过图事件触发则不会走这一套机制——这算是我特意留的漏洞。

附带一个力竭系统：任何角色将耐力消耗到 0 时会进入力竭状态，在耐力百分比恢复到默认值 20% 时退出。默认力竭状态下 NPC 无法进攻，玩家可以。力竭状态下伤害会被降低，受到的伤害会增加，受击会直接退出力竭状态。

所有数值都可以在 Settings.json 中调整。

---

韧性模块

每次攻击命中都会造成一定的韧性损伤。这和架势不同——韧性是你的"站稳能力"，清空后就会触发硬直。

不必担心打不出硬直。武器的韧性伤害和角色的韧性值都经过了系统性数值设计。你会得到像艾尔登法环一样流畅的硬直反馈：轻击削韧、重击破韧、弹反瞬间清空。

轻甲和重甲的每个部位（头/身/手/脚）都独立提供韧性加成。重甲比轻甲更难被打出硬直。

---

架势模块（失衡）

架势是一次战斗中"站得住多久"的问题。最大架势由生命值、耐力值和种族基础值共同决定。每次受到攻击都会削减架势，架势归零时触发失衡破防硬直。

不同攻击类型的架势伤害不同，同样的，每种攻击的架势伤害倍率都可以进行配置。

失衡硬直触发之后，原本攻击造成的硬直必定会触发，同时表现上升一级。大硬直级别的可以击倒敌人，和埃尔登法环类似。失衡后可以进行处决，但目前处决系统不完善。

护甲能减轻架势伤害。架势条通过 TrueHUD 可视化显示，你可以直观地看到自己和敌人的防守还能撑多久。

---

硬直模块

硬直系统在韧性之上提供了更丰富的受击反馈。本模组支持多个层级的硬直：

基于 Modern Stagger Lock 的硬直等级，可以分成

1. 小硬直
2. 中硬直
3. 大硬直
4. 特殊硬直

前三种硬直可以由韧性触发，最后的级别只会被特殊的攻击和特殊的状态（比如失衡或者破防）触发。

前三种硬直恢复时间可配置。同等级硬直再次触发时刷新恢复而不会重复播放动画，反而会加速硬直的恢复，从而做到受击脱离。
更高等级的硬直不会被低等级覆盖。特殊硬直级别不会相互覆盖，恢复的时间由硬直动画本身定义。

---

格挡模块

格挡是本模组最完善的模块之一。每把武器都有独立的格挡强度——盾牌最高，匕首最低。

限时格挡（Timed Block）：格挡开始后 150ms 内为窗口期，成功后极大幅度的格挡强度加成，并且会免疫架势崩溃。如果操作得当，完全可以像只狼那样弹刀。

弹反（Parry）：只能由玩家触发。玩家的格挡猛击被重新设计成了弹反，是一种纯粹的防御性动作，没有任何攻击判定。
弹反成功后直接清零本次伤害，并给予攻击者破防级别的硬直。

格挡强度会影响此次伤害减免的比例、反馈给攻击者的架势伤害、以及扣除的耐力。注意每次格挡消耗的耐力是有上限的（默认 50% 最大耐力值），因此不会出现满耐力格挡却清空耐力条的情况。
但与此同时，没有被格挡的伤害会照常应用。

以上数值均可以自由配置。

---

战技模块

战技是分配在武器上的特殊技，消耗魔力释放。每把武器可以通过 PrismaUI 菜单绑定一个战技，菜单在打开背包时，按下快捷键（默认为 T）显示在屏幕右侧。
战技需要消耗战技点解锁。战技点会在战技等级提升时获取，初始 3 点，每升一级获得 3 点。使用战技造成伤害会获取战技经验。战技的数值均可以配置。

在战斗中，默认按下 Alt 启用战技状态，此时的轻攻击或重攻击会变成战技。
如果魔法值足够，战技才会发挥完整威力；如果魔法值不够，战技几乎相当于普通攻击。

战技的法术释放可以在战技配置中定义，然后使用 Cast 注释触发。战技可以指定基于的技能等级、标准强度以及缩放因子——
例如指定 OneHanded 单手技能，在技能为 25 时释放的法术强度等于标准强度。因子的数值越大，前后期的伤害差距越大。

另外注意，战技的动画基于 BFCO 进行调整。如果使用 MCO，你需要自己调整一次。

---

处决模块

尚未完成。代码保留，等待后续更新。

---

对于模组开发者

环战斗提供了一套逻辑自洽的图事件（Animation Graph Event）和 Payload 注释系统。你可以在动画的 annotation 帧中发送以下事件，由环战斗接管对应的数值管线：

■ RimStamina —— 耐力管线
  Start：启用 Rim 耐力系统。启用后当前的耐力系统不会再处理此角色的攻击耐力消耗。
  End：退出 Rim 耐力系统，恢复模组默认的耐力系统。
  Consume|AttackType|Side|Multiplier|FallbackMultiplier：
    消耗耐力。AttackType = Normal 或 Power。Side = Left、Right 或 Auto。
    Multiplier 为足魔时的倍率，FallbackMultiplier 为魔力不足时的惩罚倍率。

  示例：RimStamina.Consume|Power|Right|10.0|5.0
  表示基于重击和右手武器。如果位于战技状态且战技耗魔达标，则使用 10.0 的倍率；
  耗魔不达标则使用 5.0 的倍率。如果不在战技中则始终使用 10.0，此时可以不写 Fallback。

■ RimDamage —— 伤害管线
  SetMult|Multiplier|FallbackMultiplier：
    设置本次攻击的最终伤害倍率。叠加于全局倍率之上。
  End：清除伤害倍率缓存。

  示例：RimDamage.SetMult|10.0|5.0
  效果与 RimStamina.Consume 的 Eligible/Subordinate 逻辑相同。

■ RimPosture —— 架势管线
  Damage|Multiplier|FallbackMultiplier：
    设置本次攻击的架势伤害倍率。
  End：清除架势倍率缓存。
  Unbreakable|Duration：在指定毫秒内免疫架势崩溃。

■ RimPoise —— 韧性管线
  TargetSet|Multiplier|FallbackMultiplier：
    设置击中敌人时，对敌人造成的韧性伤害倍率。
  TargetEnd：清除攻击方韧性倍率缓存。
  Set|Multiplier：设置自身受到的韧性伤害倍率。
  End：清除自身韧性倍率缓存。

■ RimStagger —— 硬直管线
  TargetSet|Level：
    为本次攻击附加指定的硬直等级。
  TargetEnd：清除附加硬直。
  Immune|Duration：在指定毫秒内免疫可被抵抗的硬直等级。只能抵抗非特殊硬直级别。

  硬直等级定义：

  None       = 0   无
  Small      = 1   小硬直
  Medium     = 2   中硬直
  Large      = 3   大硬直
  Largest    = 4   最大常规硬直（此等级以上均为特殊硬直）
  Knockaway  = 100 击飞
  Knockdown  = 101 砸趴
  Strikefly  = 102 挑飞
  GuardBreak = 200 防御崩溃（由弹反或格挡耐力耗尽触发）
  Execution  = 254 处决（最高优先级，无法被免疫）
  PostureBreak = 255 架势崩溃（架势值被打空触发）

  示例：RimStagger.TargetSet|100 —— 附加击飞硬直

■ RimWeaponArt —— 战技管线
  Start|ManaCost|MinMana：
    进入战技状态。消耗 ManaCost 点魔力。当前魔力低于 MinMana 时
    进入 Subordinate 模式（不扣魔，所有数值使用 Fallback 倍率）。
  End：退出战技状态，清除所有子系统的缓存。
  PrepareEnd：准备动画结束，进入启用状态。
  ToPrepare：进入准备状态。
  Cast|SpellName：释放战技配置中定义的法术。

■ RimBlock —— 格挡管线
  GP|Duration|Level|AutoAttack|NextAttack：
    进入精确格挡状态。在此状态内受击会给予攻击者一个硬直，并且判定为限时格挡。
    Level = 硬直等级。Duration = 持续时间（毫秒）。
    AutoAttack = true/false，成功 GP 后是否自动反击。
    NextAttack = N2 表示使用第二段轻击，P4 表示第四段重击，0 表示不修改攻击段数。
    若 AutoAttack 为 true 但 NextAttack 为 0，则不进行自动反击。

  Parry|Duration：
    进入弹反状态。在此状态内受击会给予攻击者 GuardBreak 级别的硬直。
    Duration = 持续时间（毫秒）。

注意：以上所有涉及持续时间的参数，单位均为毫秒。

---

前置与安装

必须安装：
- SKSE 1.6.629+
- Address Library
- OAR（Open Animation Replacer）
- MCO 或 BFCO（二选一）

推荐安装：
- PrismaUI（战技菜单与 HUD）
- TrueHUD（架势条）
- Modern Stagger Lock（硬直动画兼容）
- OCF（更细武器分类）
- Precision（精确碰撞检测）
- Behavior Data Injector（战技动画注入）

安装：Mod 管理器安装，dist/ 对应游戏根目录。进游戏后在 SKSEMenuFramework 菜单中确认各模块已启用。
重置设置：将 SettingsDefault.json 复制覆盖 Settings.json。

---

已知问题

- NPC 暂不能使用战技
- 处决系统暂缓
- 数值为首次系统性设计，需要实测反馈来调优
- 平衡性处于 Pre-Alpha 阶段

---

致谢

CommonLibSSE-NG / MCO / BFCO / OAR / Maxsu Poise / Modern Stagger Lock / Elden Rim

---

# English Version

Rim Combat is a melee combat overhaul for Skyrim SE/AE. It doesn't add flashy attack animations — it rebuilds the combat decision-making layer to work with modern animation frameworks like MCO or BFCO, where combat often boils down to stat-checking or dodge-rolling.

Through OCF classification, Rim Combat supports detailed per-type values for 20+ weapon types and 50+ creature races.

Seven independent, toggleable modules:

---

Stamina

Every swing costs stamina. Vanilla attack stamina costs are fully replaced. Normal attacks, power attacks, bashes, and power bashes all have their own consumption rules. Every weapon type — including mod-added subtypes — has its own base cost. Creature attacks (claws, bites, charges) use race-specific values.

Includes a powerful attack-disable mechanism: any actor at zero stamina cannot attack, preventing infinite combos. Attacks triggered directly through graph events bypass this — an intentional loophole.

Also includes an Exhausted system: when stamina hits zero, the actor enters an exhausted state, exiting when stamina recovers past 20%. By default, exhausted NPCs cannot attack (players can). Exhausted actors deal reduced damage, take increased damage, and exit exhaustion immediately upon being hit.

All values configurable via Settings.json.

---

Poise

Each hit deals poise damage — your ability to stand your ground. When poise breaks, a stagger occurs. Weapon poise damage and character poise health are systematically balanced for Elden Ring-like stagger flow: light hits chip, heavy hits break through, and parries instantly clear poise.

Light and heavy armor provide independent per-slot poise bonuses (head/body/hands/feet). Heavy armor makes you significantly harder to stagger.

---

Posture

Posture is your ability to hold your stance over the course of a fight. Max posture is determined by health, stamina, and race. Each hit drains posture; when it reaches zero, a guard-break stagger occurs.

Different attack types deal different posture damage — all configurable. After a posture break, the triggering hit's stagger is guaranteed and upgraded by one tier. At the Large tier, enemies can be knocked down, similar to Elden Ring. Posture breaks create execution openings, though the execution system is not yet complete.

Armor reduces incoming posture damage. A TrueHUD bar visualizes posture for both you and your enemies.

---

Stagger

Building on Modern Stagger Lock, the stagger system provides four tiers:

1. Small
2. Medium
3. Large
4. Special

The first three are triggered by poise breaks. Special stagger levels are reserved for specific attacks and states such as posture breaks or guard breaks.

Recovery times for the first three tiers are configurable. Re-applying the same tier refreshes recovery without replaying the animation and accelerates recovery — enabling hitstun escape. Higher tiers override lower ones. Special stagger levels do not override each other; their recovery is defined by the animation itself.

---

Block

One of the most polished modules. Every weapon has its own block strength — shields are the highest, daggers the lowest.

Timed Block: a 150ms window after starting a block. Success grants a massive block strength boost and immunity to posture breaks. With good timing, you can deflect like Sekiro.

Parry: player-only. Power bash has been redesigned as a parry — a purely defensive action with no attack hitbox. A successful parry nullifies all damage from the incoming hit and applies a guard-break stagger to the attacker.

Block strength determines damage reduction, posture damage reflected back to the attacker, and stamina drained. Stamina drain per block is capped (default 50% of max stamina), so you won't lose your entire bar from one block. Unblocked damage still applies normally.

All values are configurable.

---

Weapon Arts

Weapon Arts are special techniques assigned to weapons, consuming magicka. Each weapon can be bound to one art via the PrismaUI menu, accessible by pressing the hotkey (default: T) while the inventory is open.

Arts are unlocked with points earned from leveling up. You start with 3 points and gain 3 per level. Dealing damage with arts grants XP.

In combat, press Alt (default) to enter Weapon Art stance — your light or heavy attack becomes the assigned art. With enough magicka, the art performs at full power. Without enough magicka, it's barely better than a normal attack.

Spell casting for arts is defined in the art's configuration and triggered via the Cast annotation. Each spell can be tied to a skill level, a standard magnitude, and a scaling factor —
for example, specifying OneHanded skill means the spell's magnitude equals the standard magnitude at skill level 25. A larger factor value creates a wider power gap between early and late game.

Note: Weapon Art animations are tuned for BFCO. If using MCO, you will need to adjust them yourself.

---

Execution

Not yet complete. Code retained for future updates.

---

For Mod Developers

Rim Combat provides a coherent set of animation graph events and payload annotations. Add these events to your animation annotation frames to let Rim Combat handle the corresponding value pipeline:

■ RimStamina — Stamina pipeline
  Start: enable Rim stamina system. Once enabled, the default stamina system will no longer process attack stamina for this actor.
  End: exit Rim stamina system, restore the default stamina system.
  Consume|AttackType|Side|Multiplier|FallbackMultiplier:
    Consume stamina. AttackType = Normal or Power. Side = Left, Right, or Auto.
    Multiplier applies with sufficient magicka; FallbackMultiplier applies without.

  Example: RimStamina.Consume|Power|Right|10.0|5.0
  Based on a power attack with the right-hand weapon. If in a Weapon Art state with sufficient magicka, uses 10.0×.
  If magicka is insufficient, uses 5.0×. If not in a Weapon Art state, always uses 10.0× — Fallback can be omitted in this case.

■ RimDamage — Damage pipeline
  SetMult|Multiplier|FallbackMultiplier:
    Set final damage multiplier for this attack. Stacks on top of global multipliers.
  End: clear damage multiplier cache.

  Example: RimDamage.SetMult|10.0|5.0
  Same Eligible/Subordinate logic as RimStamina.Consume.

■ RimPosture — Posture pipeline
  Damage|Multiplier|FallbackMultiplier:
    Set posture damage multiplier for this attack.
  End: clear posture multiplier cache.
  Unbreakable|Duration: immune to posture break for the specified duration in milliseconds.

■ RimPoise — Poise pipeline
  TargetSet|Multiplier|FallbackMultiplier:
    Set poise damage multiplier applied when hitting an enemy.
  TargetEnd: clear attacker poise multiplier cache.
  Set|Multiplier: set poise damage multiplier for damage taken by self.
  End: clear self poise multiplier cache.

■ RimStagger — Stagger pipeline
  TargetSet|Level:
    Attach a stagger level to this attack.
  TargetEnd: clear attached stagger.
  Immune|Duration: immune to resistible stagger levels for the specified duration in milliseconds.
    Only resists non-special stagger levels.

  Stagger level reference:

  None       = 0
  Small      = 1
  Medium     = 2
  Large      = 3
  Largest    = 4   (everything above is special)
  Knockaway  = 100 (launch)
  Knockdown  = 101 (slam down)
  Strikefly  = 102 (pop up)
  GuardBreak = 200 (guard break — triggered by parry or block stamina depletion)
  Execution  = 254 (execution — highest priority, cannot be immunized)
  PostureBreak = 255 (posture break — triggered when posture reaches zero)

  Example: RimStagger.TargetSet|100 — attach Knockaway stagger

■ RimWeaponArt — Weapon Art pipeline
  Start|ManaCost|MinMana:
    Enter Weapon Art state. Consumes ManaCost magicka. If current magicka is below MinMana,
    enters Subordinate mode (no magicka cost, all values use Fallback multipliers).
  End: exit Weapon Art state, clear all subsystem caches.
  PrepareEnd: preparation animation complete, enter enabled state.
  ToPrepare: enter preparation state.
  Cast|SpellName: cast a spell defined in the Weapon Art configuration.

■ RimBlock — Block pipeline
  GP|Duration|Level|AutoAttack|NextAttack:
    Enter Guard Point state. Taking a hit during this state applies a stagger to the attacker
    and counts as a timed block.
    Level = stagger level. Duration = duration in milliseconds.
    AutoAttack = true/false, whether to auto-counter after a successful GP.
    NextAttack = N2 for the second light attack chain, P4 for the fourth power attack chain,
    0 to leave the attack chain unchanged. If AutoAttack is true but NextAttack is 0, no auto-counter occurs.

  Parry|Duration:
    Enter parry state. Taking a hit during this state applies a GuardBreak stagger to the attacker.
    Duration = duration in milliseconds.

Note: all duration parameters above are in milliseconds.

---

Requirements

Hard dependencies:
- SKSE 1.6.629+
- Address Library
- OAR (Open Animation Replacer)
- MCO or BFCO

Recommended:
- PrismaUI (art menu & HUD)
- TrueHUD (posture bar)
- Modern Stagger Lock (stagger compatibility)
- OCF (finer weapon subtypes)
- Precision (accurate hit detection)
- Behavior Data Injector (art animation injection)

Install with any mod manager. The dist/ folder mirrors the game root. Enable each module in the
SKSEMenuFramework menu after starting the game. To reset settings, copy SettingsDefault.json
over Settings.json.

---

Known Issues

- NPCs cannot use Weapon Arts yet
- Execution system on hold
- Values are a first-pass design and need playtesting feedback
- Pre-Alpha balance

---

Credits

CommonLibSSE-NG / MCO / BFCO / OAR / Maxsu Poise / Modern Stagger Lock / Elden Rim
