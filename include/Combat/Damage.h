#pragma once

class Damage
{
public:
  // 图事件，payload用于传递信息
  // SetMult|Multiplier用于设置伤害倍率
  // Multiplier是伤害倍率，不处理小于0的值
  // End表示此攻击的伤害倍率设置结束，清空缓存的倍率，通常在此次攻击的命中帧结束时触发
  constexpr static std::string_view RIMDAMAGE = "RimDamage";

  static Damage& GetSingleton()
  {
    static Damage singleton;
    return singleton;
  }

  // Damage系统作用于近战的初始伤害倍率，而非最终伤害倍率
  static void ProcessDamage(RE::Actor* aggressor, float& damage);

  // 在原版系统中，单武器武器重击的伤害是2倍
  // 双持重击是1.5倍伤害
  // 对于MCO框架，任何重击都是单武器重击
  // 对于BFCO框架，存在双持重击的概念，但目前极少使用
  // Bash具有0.5倍伤害
  // PowerBash具有1.5倍伤害
  // 空手重击是1倍伤害
  // 在这里对伤害进行归一化并提供统一的相对1倍倍率的数值调整
  static void ProcessWeaponDamage(RE::Actor* aggressor, RE::HitData& hitData);

  static void SetMult(RE::Actor* actor, float multiplier);

  static void SetMult(RE::Actor* actor, const std::string& payload);
  static void End(RE::Actor* actor);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Damage();
  // Rim Combat Damage System
  constexpr static inline std::uint32_t serialType = 'RCDS';

  // 伤害倍率缓存，键为Actor指针，值为当前攻击的伤害倍率，战技中根据条件使用不同的倍率
  static inline std::mutex mtx_damageMultiplier;
  static inline std::unordered_map<RE::Actor*, float> damageMultOnAttack;
};