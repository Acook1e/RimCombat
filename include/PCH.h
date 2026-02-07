#pragma once

#include "RE/Skyrim.h"
#include "REL/REL.h"
#include "REX/REX.h"
#include "SKSE/SKSE.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <deque>
#include <format>
#include <mutex>
#include <ranges>
#include <set>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <vector>

constexpr uint32_t nexusID = 2154800;

namespace logger = SKSE::log;