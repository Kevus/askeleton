#pragma once

#include <optional>
#include <string_view>

enum class CoverageMode {
    Strict,
    Balanced,
    Aggressive,
};

std::optional<CoverageMode> parseCoverageMode(std::string_view value);
const char *coverageModeName(CoverageMode mode);
