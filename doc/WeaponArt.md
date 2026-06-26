# WeaponArt · 战技系统

## 定位

玩家专属的武器必杀技系统。每把武器绑定一个战技，按下指定按键进入准备/发动状态。战技定义包含多阶段连段攻击、魔力阈值双模式、绑定技能法术释放。全 JSON 驱动，动画注解控制状态机。

---

## 状态机

| 状态 | 枚举值 | 触发 | 含义 |
|------|:---:|------|------|
| `Disable` | 0 | 默认 / End 事件 | 战技未激活 |
| `Prepare` | 1 | `ToPrepare` 事件（需 `needPrepare=true`） | 拔刀/前置动画中 |
| `Enable` | 2 | `PrepareEnd` 事件 / 无前置时直接 | 战技发动中，可连段 |

收刀时自动切回 `Disable`。

---

## Stage / Attack 层级

```
战技
  ├─ Stage 1: { manaCost, minMana, attacks: { ... } }
  │   ├─ Attack 1.1: 8 个乘数 (Eligible + Subordinate 各 4 个)
  │   ├─ Attack 1.2: ...
  │   └─ Attack 1.N: ...
  ├─ Stage 2: { manaCost, minMana, attacks: { ... } }
  └─ ...
```

每个 Stage 有独立魔力消耗和攻击帧数。Attack 帧定义左右手/强力攻击标志及 8 个浮点乘数。

---

## Eligible / Subordinate 双模式

进入 Stage 时检测魔力：

| 条件 | 模式 | 效果 |
|------|------|------|
| 魔力 ≥ `minMana` | **Eligible** | 扣 `manaCost` 魔力，全部乘数用 Eligible 组 |
| 魔力 < `minMana` | **Subordinate** | 不扣魔力，全部乘数用 Subordinate 组（降级） |

两组各 4 个乘数分别控制：

| 乘数 | 影响子系统 |
|------|------|
| `staminaMult` / `subStaminaMult` | 耐力消耗 |
| `damageMult` / `subDamageMult` | 基础伤害 |
| `poiseDamageMult` / `subPoiseDamageMult` | 韧性伤害 |
| `postureDamageMult` / `subPostureDamageMult` | 架势伤害 |

---

## 法术系统

战技可携带任意数量的法术。每个法术定义：

| 字段 | 说明 |
|------|------|
| `mod` | 法术所在插件（`.esp`/`.esm`） |
| `formID` | 十六进制 FormID |
| `effectiveness` | 效果倍率 |
| `stdMagnitude` | 基准强度（技能 25 时的值） |
| `factor` | 缩放曲线的陡度因子 |
| `skill` | 缩放绑定的技能（18 种可选） |
| `selfCast` | 是否自施法（true=自身, false=目标） |

**强度公式**：`magnitude = stdMagnitude × skillMult × (skillLevel ÷ 25)^(factor ÷ (factor + 10))`

---

## 武器匹配

战技通过 20 位标志域匹配武器类型：

| 分类 | 标志 |
|------|------|
| **按重量** | `Light`, `Normal`, `Heavy` |
| **按攻击类型** | `Slash`, `Thrust`, `Blunt`, `Polearm`, `Range` |
| **按武器家族** | `Sword`, `Axe`, `Hammer`, `Fist`, `Katana`, `Spear`, `Stick`, `Whip`, `Bow`, `Crossbow` |
| **特殊** | `Unique` — 配合 `weapons` 字段按 FormID 精确匹配 |

解析时按 OCF 关键词和原版 `WEAPON_TYPE` 回退判定。

---

## 进度系统

| 机制 | 说明 |
|------|------|
| **经验** | 玩家在战技发动期间（Eligible 或 Subordinate）命中目标时获得。按最终伤害比例结算 |
| **等级** | 经验累积升级，`cap=100` |
| **点数** | 初始 3，每级 +1。解锁战技需要消耗点数 |
| **解锁条件** | `ownAtStart=true`（初始拥有），或达到 `consumePoint` + `unlockLevel` 后手动解锁 |

进度通过 SKSE 序列化持久化。

---

## UI

| 组件 | 说明 |
|------|------|
| **HUD** | PrismaUI 浮层，显示当前战技名 + 状态文字。可调位置/缩放，收刀时隐藏 |
| **菜单** | PrismaUI 全屏目录，列出已拥有战技。显示等级/点数。支持绑定/解绑武器 |
| **菜单联动** | 战技菜单与物品栏菜单联动 — 选中背包中的武器自动高亮其绑定的战技 |

---

## 动画事件

通过 `RimWeaponArt` 图事件控制状态机：

| Payload | 效果 |
|------|------|
| `Stage\|N` | 进入第 N 阶段 — 检查魔力，设置 Eligible/Subordinate |
| `Attack\|N\|M` | 发起第 N 阶段第 M 次攻击 — 应用对应乘数到四个子系统 |
| `End` | 结束战技，恢复 Disable，清除子系统缓存 |
| `Cast\|Name` | 释放名称为 Name 的法术 |
| `ToPrepare` | 开始准备状态（需 `needPrepare=true`） |
| `PrepareEnd` | 准备完成，进入 Enable |

---

## JSON 配置

- **位置**：`WeaponArt/*.json`
- **Schema**：`scripts/WeaponArt.schema.json`（VS Code 自动校验）
- **格式**：顶层 key 为战技逻辑名（hash 后用于运行时 ID）

---

## 设置入口

| 设置键 | 说明 |
|------|------|
| `bUseWeaponArtSystem` | 总开关 |
| `bUseWeaponArtHUD` | HUD 开关 |
| `bHideWeaponArtHUDOnSheathe` | 收刀隐藏 HUD |
| `fWeaponArtHUDPosX/Y` | HUD 位置 |
| `fWeaponArtHUDScale` | HUD 缩放 |
| `fWeaponArtMenuStartPercent` | 菜单起始位置百分比 |
