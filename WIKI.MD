# Rim Combat Wiki

> This wiki is based on the current codebase, not on an older design draft.
>
> Progress notes below exclude the execution system by default. Execution still exists in the code, but active development is currently paused because of OAR-side constraints.

---

## 1. Overview

Rim Combat is a melee combat rebuild for Skyrim SE/AE that focuses on commitment, pressure, and controllable combat pacing instead of only adding flashy attack strings.

The current playable loop is built around these layers:

- weapon-type aware data
- attack stamina and exhausted state
- posture break pressure
- internal poise and stagger handling
- block and timed block mindgames
- weapon art assignment, state switching, and HUD/menu support

At the current stage, the mod is already suitable for sustained playtesting and balance iteration, but it is still a pre-release prototype rather than a finished combat overhaul.

---

## 2. Before vs After Enabling Rim Combat

| Area | Vanilla / Minimal Setup | With Rim Combat |
| --- | --- | --- |
| Attack resource loop | Attacks largely follow vanilla stamina behavior | Stamina cost is driven by weapon type, weapon mass, attack type, Precision events, and optional animation payloads |
| Stamina failure | Running low on stamina is often just a soft penalty | Zero stamina can fully lock attacks if enabled, and exhausted state can further punish offense and defense |
| Defensive play | Blocking is mostly damage reduction | Block strength, stamina drain, posture interaction, and timed block windows all matter |
| Break pressure | No dedicated posture resource | Posture becomes a separate combat resource with per-race bases, regen delay, and break handling |
| Hit reaction | Vanilla stagger only | Internal poise damage, threshold-based stagger levels, recovery timing, and special stagger extensions work together |
| Weapon abilities | Static skill/perk flow | Weapon arts can be unlocked, assigned per weapon, toggled, prepared, and extended with spell payloads |
| UI and tuning | Mostly static | JSON-driven settings, localization, HUD state display, and a right-side inventory companion menu |

---

## 3. Active Systems

### Weapon Typing And Data Layer

- The plugin classifies weapons beyond the vanilla broad categories.
- With OCF installed, Rim Combat can use finer subtype detection for swords, katanas, spears, whips, quarterstaves, and more.
- Core per-weapon values such as stamina cost, posture damage, poise damage, block strength, and execution multipliers are all keyed by weapon type.
- Without OCF, the system still works, but classification falls back to a coarser vanilla-oriented path.

### Attack Stamina And Exhausted State

- Vanilla swing stamina, Precision collision events, and Rim Combat payload events all funnel into one stamina system.
- Separate settings exist for normal attacks, power attacks, weapon mass scaling, stamina regeneration, combat regeneration, and block regeneration.
- Exhausted state can disable attacks, alter outgoing damage, increase incoming punishment, and optionally clear itself on hit.
- NPCs and the player can be configured independently for exhausted attack lockout.

### Block And Timed Block

- Blocking has a tracked start time and a timed block window.
- Timed block can grant a short protected duration, improved block strength, and optional immunity against posture break.
- Block processing modifies damage taken and posture impact instead of acting as a purely cosmetic flag.
- The block system is one of the most complete slices in the current build.

### Posture

- Max posture is derived from health, stamina, and race-based defaults.
- Posture damage is driven by weapon type and attack kind.
- Bash, power attack, and power bash each have their own posture multipliers.
- Armor reduces posture damage through a dedicated factor.
- The posture HUD path currently targets TrueHUD.

### Poise And Stagger

- Poise is an internal resource used to determine stagger severity.
- Max poise is influenced by stamina, equipment mass, armor class bonuses, and race-based overrides.
- Poise damage is weapon-driven and supports separate self and target multipliers through animation payloads.
- Stagger has configurable recovery times and can also be extended with special levels such as knockdown, strikefly, and knockaway.
- Stagger application now keeps the higher hit level instead of letting a weaker target payload overwrite a stronger existing level.
- Reapplying the same stagger level can extend recovery timing without replaying the same stagger animation.
- Projectile or spell-like stagger sources can be authored through JSON files in the stagger directory.

### Weapon Arts

- Weapon arts are loaded from JSON files and hashed into runtime IDs.
- A weapon art can be assigned per weapon and can also support race-based special unarmed-form IDs such as human unarmed, werewolf, werebear, and vampire lord.
- Each art can require unlock level, consume weapon-art points, optionally use a prepare state, and define spell payloads.
- Player progression currently grants experience from weapon-art damage and awards points on level-up.
- The player-facing portion and special creature-form interface are functional now; NPC-side AI usage and content breadth are still incomplete.

### UI, Localization, And Settings

- PrismaUI is used for the weapon-art menu and HUD.
- The assignment menu occupies the right side of the screen while inventory is open and refreshes against the currently selected weapon.
- HUD state and weapon-art names are updated directly from the native side.
- Settings are grouped into JSON sections and can be reset by overwriting `Settings.json` with `SettingsDefault.json`.
- Localization text is stored separately and resolved before being sent to the UI.

### Execution Status

- Execution hooks, state, and payload routing still exist in the codebase.
- It is intentionally excluded from current completion estimates.
- Current documentation treats execution as paused work, not as a fully supported player-facing feature.

---

## 4. Controls, Files, And Compatibility

### Current Controls

- Press `T` while the inventory menu is open to toggle the weapon-art assignment menu.
- Press `Left Alt` to switch weapon-art state on the player.

### Runtime Files

- `Data/SKSE/Plugins/RimCombat/Settings.json`: active runtime settings
- `Data/SKSE/Plugins/RimCombat/SettingsDefault.json`: reset source used by `ResetSettings()`
- `Data/SKSE/Plugins/RimCombat/WeaponArt/*.json`: weapon-art definitions
- `Data/SKSE/Plugins/RimCombat/Stagger/*.json`: projectile or spell stagger mappings
- `Data/SKSE/Plugins/RimCombat/Localization.json`: UI/localization text

### Recommended Mod Stack

- Required: SKSE
- Recommended for animation flow: OAR, MCO or BFCO
- Recommended for UI: PrismaUI
- Recommended for posture feedback: TrueHUD
- Recommended for the intended stagger ecosystem: Maxsu Poise and Modern Stagger Lock
- Optional: OCF for finer weapon typing

---

## 5. Settings Reference

### General Notes

- `ResetSettings()` overwrites `Settings.json` from `SettingsDefault.json`, then reloads and reapplies the settings.
- The current code only loads the keys documented below.
- Weapon-keyed maps use weapon enum names such as `Dagger`, `Sword`, `Rapier`, `Katana`, `GreatSword`, `Pike`, `Bow`, `Shield`, and `Torch`.
- Race-keyed maps use the race enum names already generated into the shipped default file. In practice, the safest workflow is to edit those existing entries in place.
- Legacy aliases `ImpactLevelSmall`, `ImpactLevelMedium`, and `ImpactLevelLarge` are still accepted when loading poise thresholds, but saves write the newer `StaggerLevel*` names.
- If your shipped sample file still contains older execution keys that are not listed below, those keys are legacy and are ignored by the current loader.

### Stamina

| Key | Purpose |
| --- | --- |
| `UseAttackStaminaSystem` | Master switch for the custom attack stamina system |
| `ConsumeStaminaOutCombat` | Allows the custom stamina flow outside combat contexts |
| `DisableAttackWhenStaminaZero` | Prevents attacks when stamina reaches zero |
| `StaminaRegenMult` | Global stamina regeneration multiplier |
| `StaminaRegenMin` | Lower clamp for regenerated stamina rate |
| `StaminaRegenMax` | Upper clamp for regenerated stamina rate |
| `StaminaRegenDelay` | Delay before stamina regen resumes after taking stamina damage |
| `StaminaRegenMultCombat` | Combat-only stamina regeneration multiplier |
| `StaminaRegenMultBlock` | Regeneration multiplier while blocking |
| `NormalAttackStaminaCostPerMass` | Extra normal attack stamina cost per weapon mass unit |
| `PowerAttackStaminaCostMult` | Multiplier applied to base stamina cost for power attacks |
| `PowerAttackStaminaCostPerMass` | Extra power attack stamina cost per weapon mass unit |
| `BaseStaminaCost` | Base stamina cost map keyed by weapon type |

### Posture

| Key | Purpose |
| --- | --- |
| `UsePostureSystem` | Master switch for posture |
| `UsePostureHUD` | Enables posture HUD output, intended for TrueHUD integration |
| `MaxPostureHealthMult` | Extra max posture gained per point of max health |
| `MaxPostureStaminaMult` | Extra max posture gained per point of max stamina |
| `PostureRegenDelay` | Delay before posture starts regenerating again |
| `PostureRegenPercentPerSecond` | Percent of max posture regenerated per second |
| `BashPostureDamageMult` | Posture multiplier for bashes |
| `PowerAttackPostureDamageMult` | Posture multiplier for power attacks |
| `PowerBashPostureDamageMult` | Posture multiplier for power bashes |
| `ArmorPostureDamageFactor` | Armor-based reduction factor for incoming posture damage |
| `BasePosture` | Base max posture map keyed by race type |
| `BasePostureDamage` | Base posture damage map keyed by weapon type |

### Poise

| Key | Purpose |
| --- | --- |
| `UsePoiseSystem` | Master switch for poise |
| `PoiseStaminaMult` | Max poise gained per point of max stamina |
| `PoiseMassMult` | Max poise gained from equipment mass |
| `PoiseRegenDelay` | Delay before poise regeneration resumes |
| `PoiseRegenPercentPerSecond` | Percent of max poise regenerated per second |
| `StaggerLevelSmall` | Poise damage threshold for small stagger |
| `StaggerLevelMedium` | Poise damage threshold for medium stagger |
| `StaggerLevelLarge` | Poise damage threshold for large stagger |
| `LightArmorHeadMaxPoiseBonus` | Max poise bonus from light armor head slot |
| `LightArmorBodyMaxPoiseBonus` | Max poise bonus from light armor body slot |
| `LightArmorHandMaxPoiseBonus` | Max poise bonus from light armor hand slot |
| `LightArmorFeetMaxPoiseBonus` | Max poise bonus from light armor feet slot |
| `HeavyArmorHeadMaxPoiseBonus` | Max poise bonus from heavy armor head slot |
| `HeavyArmorBodyMaxPoiseBonus` | Max poise bonus from heavy armor body slot |
| `HeavyArmorHandMaxPoiseBonus` | Max poise bonus from heavy armor hand slot |
| `HeavyArmorFeetMaxPoiseBonus` | Max poise bonus from heavy armor feet slot |
| `BashPoiseDamageMult` | Poise damage multiplier for bashes |
| `PowerAttackPoiseDamageMult` | Poise damage multiplier for power attacks |
| `PowerBashPoiseDamageMult` | Poise damage multiplier for power bashes |
| `BasePoise` | Base max poise map keyed by race type |
| `BasePoiseDamage` | Base poise damage map keyed by weapon type |

### Stagger

| Key | Purpose |
| --- | --- |
| `UseStaggerSystem` | Master switch for the stagger extension layer |
| `StaggerRecoveryTimeSmall` | Recovery window for small stagger |
| `StaggerRecoveryTimeMedium` | Recovery window for medium stagger |
| `StaggerRecoveryTimeLarge` | Recovery window for large stagger |

### Exhausted

| Key | Purpose |
| --- | --- |
| `UseExhaustedSystem` | Master switch for exhausted state |
| `DisablePlayerAttackWhenExhausted` | Blocks player attacks while exhausted |
| `DisableNPCAttackWhenExhausted` | Blocks NPC attacks while exhausted |
| `ExitExhaustedOnHit` | Clears exhausted state when the actor is hit |
| `ExhaustedExitPercent` | Stamina percent required to exit exhausted state |
| `AttackDamageMultWhenExhausted` | Outgoing damage multiplier while exhausted |
| `AttackPostureDamageMultWhenExhausted` | Outgoing posture damage multiplier while exhausted |
| `OnHitDamageMultWhenExhausted` | Incoming damage multiplier while exhausted |
| `OnHitPostureDamageMultWhenExhausted` | Incoming posture damage multiplier while exhausted |

### Block

| Key | Purpose |
| --- | --- |
| `UseBlockSystem` | Master switch for block handling |
| `TimedBlockEnabled` | Enables timed block logic |
| `TimedBlockNeverPostureBreak` | Prevents posture break during timed block if enabled |
| `TimedBlockLimit` | Timed block activation window |
| `TimedBlockDuration` | Duration of the timed block protected state |
| `BlockMaxStaminaConsumePercent` | Maximum stamina percent consumed by one block |
| `BlockMinStaminaConsume` | Extra minimum stamina floor used when a block strains stamina |
| `TimedBlockBlockStrengthMult` | Extra block strength multiplier during timed block |
| `BlockStrength` | Block strength map keyed by weapon type |

### WeaponArt

| Key | Purpose |
| --- | --- |
| `UseWeaponArtSystem` | Master switch for weapon arts |
| `UseWeaponArtHUD` | Enables weapon-art HUD display |
| `WeaponArtHUDPosX` | HUD horizontal position in screen percent |
| `WeaponArtHUDPosY` | HUD vertical position in screen percent |
| `WeaponArtHUDScale` | HUD scale |
| `WeaponArtMenuStartPercent` | Horizontal start position for the assignment menu |

### Execution

| Key | Purpose |
| --- | --- |
| `UseExecutionSystem` | Master switch for execution logic |
| `OnHitDamageMultWhenExecutable` | Damage multiplier applied when hitting an executable target |
| `ExecutionDamageMult` | Execution damage multiplier map keyed by weapon type |

---

## 6. Modder Notes: Animation Events And Payloads

This section is for modders, animators, and content authors. Regular players do not need any of this.

### Routing Rules

- Animation event tags and payload strings are lowercased before routing.
- One-shot combat caches are automatically cleared on `attackstop` and `staggerstart`.
- The native side already listens for vanilla and framework events such as `weaponswing`, `weaponleftswing`, `collision_attackstart`, `collision_add`, `collision_attackend`, `blockstart`, `blockstartout`, and `blockstop`.

### Rim Combat Payload Events

| Event tag | Payload syntax | Effect |
| --- | --- | --- |
| `RimStamina` | `Start` | Switches the actor into Rim Combat stamina mode for the current action |
| `RimStamina` | `End` | Ends the custom stamina handling for the current action |
| `RimStamina` | `Consume|AttackType|Side|Multiplier|FallbackMultiplier` | Applies stamina consumption based on attack type and side |
| `RimDamage` | `SetMult|Multiplier|FallbackMultiplier` | Sets the damage multiplier for the current melee action |
| `RimDamage` | `End` | Clears cached melee damage multiplier |
| `RimPoise` | `Set|Multiplier` | Applies a self poise-damage modifier |
| `RimPoise` | `End` | Clears self poise-damage modifier |
| `RimPoise` | `TargetSet|Multiplier` | Applies poise-damage modifier to the hit target |
| `RimPoise` | `TargetEnd` | Clears target poise-damage modifier |
| `RimPosture` | `Unbreakable|Duration` | Makes the actor posture-break immune for a duration in milliseconds |
| `RimPosture` | `Damage|Multiplier|FallbackMultiplier` | Sets posture damage multiplier for the current action |
| `RimPosture` | `End` | Clears cached posture multiplier |
| `RimStagger` | `TargetSet|Level` | Sets stagger level to apply on hit |
| `RimStagger` | `TargetEnd` | Clears cached target stagger level |
| `RimStagger` | `Immune|Level|Duration` | Grants stagger immunity up to the specified level for a duration in milliseconds |
| `RimWeaponArt` | `Start|ManaCost|MinMana` | Starts weapon-art execution, marks eligibility, and enables Rim Combat stamina handling |
| `RimWeaponArt` | `End` | Ends the current weapon-art action and clears related caches |
| `RimWeaponArt` | `PrepareEnd` | Moves weapon-art state from prepare to enabled |
| `RimWeaponArt` | `ToPrepare` | Forces weapon-art state into prepare |
| `RimWeaponArt` | `Cast|SpellName` | Casts a configured weapon-art spell entry |
| `RimExecution` | `Damage|Multiplier` | Experimental execution damage event |
| `RimExecution` | `VictimEnd` / `End` | Experimental execution cleanup events |

### Important Current Caveat

- The stagger enum includes `Knockdown`, `Strikefly`, and `Knockaway` as extra special levels.
- `TargetSet|Level` and `Immune|Level|Duration` can both use those extra levels.
- A weaker `TargetSet|Level` payload no longer downgrades a stronger current stagger level.
- Reapplying the same level refreshes recovery timing without forcing the same stagger animation to replay.

### Weapon-Art JSON Authoring

Weapon-art files live in `Data/SKSE/Plugins/RimCombat/WeaponArt/`.

| Field | Required | Notes |
| --- | --- | --- |
| object key | Yes | The JSON object key is hashed into the runtime weapon-art ID |
| `name` | Yes | Display name used by UI |
| `description` | No | UI description |
| `availableWeapon` | No | Array of `AvailableWeapon` enum names |
| `weapons` | No | Explicit weapon list, mainly for `Unique` arts |
| `consumePoint` | No | Weapon-art point cost to unlock |
| `unlockLevel` | No | Required weapon-art level |
| `needPrepare` | No | Whether the art uses a prepare state before fully enabling |
| `spells` | No | Named spell payload definitions used by `Cast|SpellName` |
| `verbose` | No | Prints loaded weapon-art info to the log |

Current matching rules:

- If `availableWeapon` omits weight tags, the loader expands them to a wildcard.
- If `availableWeapon` omits family tags, the loader expands them to a wildcard.
- If `availableWeapon` omits attack tags, attack matching is treated as wildcard.
- `Unique` is meant for explicit weapon assignment and should be paired with a populated `weapons` list.
- Spell keys are lowercased before hashing, so `Cast|SpellName` should match the same logical spell key regardless of case.

Minimal example:

```json
{
  "Vacuum Slash": {
    "name": "Vacuum Slash",
    "description": "Forward slash with a projectile follow-up.",
    "availableWeapon": ["Normal", "Heavy", "Slash"],
    "consumePoint": 2,
    "unlockLevel": 0,
    "needPrepare": false,
    "spells": {
      "VacuumChop": {
        "mod": "EldenSkyrim_RimSkills.esp",
        "formID": "0xBA21",
        "effectiveness": 1.0,
        "stdMagnitude": 25.0,
        "factor": 10.0,
        "skill": "OneHanded",
        "selfCast": false
      }
    }
  }
}
```

### Projectile / Spell Stagger JSON

Stagger mapping files live in `Data/SKSE/Plugins/RimCombat/Stagger/` and are arrays of objects.

Example:

```json
[
  {
    "mod": "EldenSkyrim_RimSkills.esp",
    "formID": "0xB3C5",
    "level": "Strikefly"
  }
]
```

This lets you assign a stagger level to projectile or spell-like hit sources without hardcoding them in C++.