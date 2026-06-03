#pragma once

#include "RE/Skyrim.h"
#include "REL/REL.h"
#include "REX/REX.h"
#include "SKSE/SKSE.h"

#include <algorithm>
#include <array>
#include <bit>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <deque>
#include <format>
#include <future>
#include <initializer_list>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <numbers>
#include <numeric>
#include <optional>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <variant>
#include <vector>

constexpr inline std::string_view PLUGIN_NAME = "RimCombat";
constexpr inline std::uint32_t MOD            = 'RCBT';
constexpr inline std::uint32_t nexusID        = 180357;

inline REL::Version RUNTIME = SKSE::RUNTIME_SSE_1_6_1170;

// 提前定义魔法枚举的范围，默认范围是-128~127，无法满足本项目枚举的需求
#define MAGIC_ENUM_RANGE_MIN 0
#define MAGIC_ENUM_RANGE_MAX 256

namespace logger = SKSE::log;