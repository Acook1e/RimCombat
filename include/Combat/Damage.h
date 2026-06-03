#pragma once

class Damage
{
public:
  // 图事件，payload用于传递信息
  // SetMult|Multiplier|FallbackMultiplier用于设置伤害倍率
  // Multiplier和FallbackMultiplier是伤害倍率，不处理小于0的值
  // Multiplier代表在满足战技条件下的倍率，FallbackMultiplier代表不满足战技条件下的倍率
  // 在非战技中使用Multiplier的倍率，可以不写FallbackMultiplier，因为根本不会处理
  // End表示此攻击的伤害倍率设置结束，清空缓存的Damage|Multiplier事件，通常在此次攻击的命中帧结束时触发
  constexpr static std::string_view RIMDAMAGE = "RimDamage";

  static Damage& GetSingleton()
  {
    static Damage singleton;
    return singleton;
  }

  // Damage系统作用于近战的初始伤害倍率，而非最终伤害倍率
  static void ProcessDamage(RE::Actor* aggressor, float& damage);

  // 对于MCO/BFCO框架，武器重击的伤害是两倍
  // Bash具有0.5倍伤害
  // PowerBash具有1.5倍伤害
  // 空手重击是1倍伤害
  // 在这里对伤害进行归一化并提供统一的相对1倍倍率的数值调整
  static void ProcessWeaponDamage(RE::Actor* aggressor, RE::HitData& hitData);

  static void SetMult(RE::Actor* actor, const std::string& payload);
  static void End(RE::Actor* actor);

  static void PayloadParse(RE::Actor* actor, const std::string& payload);

private:
  Damage();
  // Rim Combat Damage System
  constexpr static inline std::uint32_t serialType = 'RCDS';

  // 伤害倍率缓存，键为Actor指针，值为当前攻击的伤害倍率，战技中根据条件使用不同的倍率
  static inline std::mutex mtx_damageCache;
  static inline std::unordered_map<RE::Actor*, float> damageCache;
};