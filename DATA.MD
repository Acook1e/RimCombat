# Rim Combat Data Reference

这份文档不是“照搬法环原数值”，而是把常见法环战技转成更适合 Skyrim 底层近战系统的参考模板。

核心前提只有两条：

1. RimCombat 的 Damage / Posture / Stamina payload 都是对原版基础结果的后乘修正，不是替代原版底层伤害公式。
2. Skyrim 的轻攻击 / 重攻击、perk、锻造、附魔与难度倍率都会继续叠加，因此数值必须比法环更克制。

---

## 使用原则

### 总原则

1. 满足条件时，只强化一个主轴，最多再强化一个次轴。
2. 不满足条件时，不要完全失效，通常保留 70% 到 85% 的总价值。
3. 轻攻击派生战技更适合提高架势收益和节奏收益。
4. 重击派生战技本身已经吃到原版重击收益，payload 倍率必须更保守。
5. 不要同时给出高伤害、高架势、低耐力三赢组合。

### 三类定位

1. 输出型：优先强化生命伤害。
2. 破姿态型：优先强化架势伤害。
3. 节奏型：优先控制耐力成本与稳定命中价值。

---

## 默认模板

### 轻攻击派生战技

| 类型 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- |
| 输出型 | 1.20 | 0.95 | 1.10 | 0.95 | 1.10 | 1.00 |
| 破姿态型 | 1.00 | 0.90 | 1.40 | 1.00 | 1.15 | 1.00 |
| 节奏型 | 0.95 | 0.90 | 1.15 | 1.00 | 0.95 | 0.90 |

### 重击派生战技

| 类型 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- |
| 输出型 | 1.10 | 0.95 | 1.05 | 0.95 | 1.00 | 0.95 |
| 破姿态型 | 0.95 | 0.90 | 1.25 | 1.00 | 1.05 | 1.00 |
| Punish 型 | 1.20 | 0.95 | 1.10 | 0.95 | 1.10 | 1.00 |

说明：

1. 轻攻击派生可以更积极地给架势倍率，因为原版基础伤害较低。
2. 重击派生不要再大幅拉高伤害倍率，否则会和原版重击收益叠爆。
3. Fallback 不是失败惩罚，而是“降级版战技”。

---

## 条件强度建议

### 软条件

例子：

1. 进入战技状态。
2. 武器类型符合。
3. 动作本身正常打出。

建议差值：

1. Damage：10% 到 15%
2. Posture：15% 到 25%
3. Stamina：0% 到 10%

### 中条件

例子：

1. 从正面命中。
2. 对方未格挡。
3. 在指定窗口命中。

建议差值：

1. Damage：15% 到 30%
2. Posture：25% 到 50%
3. Stamina：5% 到 15%

### 硬条件

例子：

1. 反击窗口命中。
2. 对方处于力竭或破绽。
3. 极短 timing 条件。

建议差值：

1. Damage：30% 到 60%
2. 或者 Posture：50% 到 100%

注意：

硬条件不要同时把 Damage 和 Posture 都拉满。

---

## 法环常见战技参考

下面这些是“参考定位”，不是 1:1 复刻法环原作表现。

### 居合 / Unsheathe

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 轻派生斩击 | 轻攻击 | 输出型 | 1.25 | 1.00 | 1.15 | 1.00 | 1.05 | 1.00 |
| 重派生突刺 | 重攻击 | Punish 型 | 1.20 | 0.95 | 1.20 | 1.00 | 1.10 | 1.00 |

建议：

1. 轻派生更适合做稳定输出。
2. 重派生更适合吃正面 punish 或 counter 条件。

### 二连斩 / Double Slash

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 连续斩击 | 轻攻击 | 节奏型 | 1.10 | 0.95 | 1.20 | 1.00 | 1.00 | 0.95 |

建议：

1. 不要给太高单段伤害。
2. 价值主要来自连续命中的姿态压力。

### 回旋斩 / Spinning Slash

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 范围斩击 | 轻攻击 | 输出型 / AOE | 1.15 | 0.95 | 1.10 | 0.95 | 1.10 | 1.00 |

建议：

1. AOE 已经自带价值，倍率不要过激。
2. 多目标战技通常更应该保守伤害倍率。

### 风暴刃 / Storm Blade

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 剑气 | 轻攻击 | 节奏型 / 中距离压制 | 1.05 | 0.95 | 1.10 | 1.00 | 1.05 | 1.00 |

建议：

1. 中距离安全性已经很高，不建议再给高伤害。
2. 主要价值应来自稳定命中与节奏维持。

### 贯穿 / Impaling Thrust

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 突刺 | 重攻击 | Punish 型 | 1.20 | 1.00 | 1.10 | 1.00 | 1.10 | 1.00 |

建议：

1. 适合给前方命中或对手恢复期命中的条件奖励。
2. 不适合同时给高姿态和高伤害。

### 真空斩 / Vacuum Slash

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 远距斩波 | 重攻击 | 输出型 / 中距离 | 1.15 | 0.95 | 1.05 | 0.95 | 1.10 | 1.00 |

建议：

1. 远程/半远程战技不要给过高姿态倍率。
2. 主要卖点是覆盖范围和命中便利，不是破防。

### 猎犬步伐类机动技

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 位移 / 再起手 | 轻攻击 | 节奏型 / 机动型 | 0.95 | 0.90 | 1.00 | 0.95 | 0.90 | 0.85 |

建议：

1. 机动本身就是收益，伤害应当保守。
2. 可以用较低耐力去换位置优势。

### 狩猎巨人 / Giant Hunt

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 挑飞 punish | 重攻击 | Punish / 破姿态 | 1.15 | 0.95 | 1.35 | 1.05 | 1.15 | 1.00 |

建议：

1. 这种战技真正强的是受击表现和站位收益，不必把伤害拉太高。
2. 如果后续接硬直扩展，优先给特殊硬直，不优先加纯伤害。

### 狮子斩 / Lion's Claw

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 高风险重击 | 重攻击 | Punish 型 | 1.25 | 1.00 | 1.20 | 1.00 | 1.20 | 1.05 |

建议：

1. 因为动作承诺大，可以给偏高伤害。
2. 但仍然不建议把姿态一起堆太高。

### 风暴足 / Storm Stomp

| 分支 | 派生 | 定位 | Eligible Damage | Fallback Damage | Eligible Posture | Fallback Posture | Eligible Stamina | Fallback Stamina |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| 起手压制 | 轻攻击 | 破姿态型 | 1.00 | 0.90 | 1.45 | 1.05 | 1.10 | 1.00 |

建议：

1. 这类起手控制技优先给姿态收益。
2. 命中稳定度高时要刻意压低生命伤害。

---

## 推荐的数值试调顺序

1. 先固定 Damage，再调 Posture。
2. Posture 稳定后，再调整 Stamina。
3. 不要同时改三个倍率，否则很难知道手感变化来自哪一项。
4. 优先做一套默认模板，再给个别战技做特例修正。

---

## 给当前 RimCombat 的直接建议

1. 轻攻击派生战技，优先拿姿态收益和节奏收益。
2. 重击派生战技，不要再大幅提高生命伤害倍率。
3. AOE、远距、位移型战技，伤害倍率必须更克制。
4. 如果一个战技已经给了特殊硬直、霸体或强控制，纯数值倍率就该下降。
5. 如果一个战技满足条件时已经明显强于普通攻击，Fallback 只需要保证“能用”，不需要继续很强。