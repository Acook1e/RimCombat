#pragma once

#include "Settings.h"

class Serialization
{
public:
  virtual void Save(SKSE::SerializationInterface* a_interface)   = 0;
  virtual void Load(SKSE::SerializationInterface* a_interface)   = 0;
  virtual void Revert(SKSE::SerializationInterface* a_interface) = 0;

protected:
  template <class T>
  static bool Write(SKSE::SerializationInterface* a_intfc, const T& a_val)
  {
    assert(a_intfc);
    return a_intfc->WriteRecordData(a_val);
  }

  template <class T>
  static bool Read(SKSE::SerializationInterface* a_intfc, T& a_val)
  {
    assert(a_intfc);
    a_intfc->ReadRecordData(a_val);
    return true;
  }

  // Generic unordered_map writer. Caller supplies how to write keys/values.
  template <class K, class V>
  static bool WriteUnorderedMap(SKSE::SerializationInterface* a_intfc, const std::unordered_map<K, V>& a_map,
                                const std::function<bool(SKSE::SerializationInterface*, const K&)>& a_writeKey,
                                const std::function<bool(SKSE::SerializationInterface*, const V&)>& a_writeVal)
  {
    assert(a_intfc);
    const std::size_t count = a_map.size();
    if (!Write(a_intfc, count)) {
      return false;
    }

    for (auto& [k, v] : a_map) {
      if (!a_writeKey(a_intfc, k)) {
        return false;
      }
      if (!a_writeVal(a_intfc, v)) {
        return false;
      }
    }

    return true;
  }

  // Generic unordered_map reader. Caller supplies how to read keys/values.
  template <class K, class V>
  static bool ReadUnorderedMap(SKSE::SerializationInterface* a_intfc, std::unordered_map<K, V>& a_map,
                               const std::function<bool(SKSE::SerializationInterface*, K&)>& a_readKey,
                               const std::function<bool(SKSE::SerializationInterface*, V&)>& a_readVal)
  {
    assert(a_intfc);
    std::size_t count;
    a_intfc->ReadRecordData(count);

    for (std::size_t i = 0; i < count; ++i) {
      K k;
      V v;
      if (!a_readKey(a_intfc, k)) {
        return false;
      }
      if (!a_readVal(a_intfc, v)) {
        return false;
      }
      a_map.emplace(std::move(k), std::move(v));
    }

    return true;
  }
};