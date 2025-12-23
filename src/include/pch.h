#pragma once

#include "RE/Skyrim.h"
#include "REL/REL.h"
#include "SKSE/SKSE.h"

#include <algorithm>
#include <chrono>
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

#include "setting.h"

namespace logger = SKSE::log;

using PoiseType = Settings::PoiseType;