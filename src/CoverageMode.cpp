#include "CoverageMode.hpp"

#include <string>

#include "utils/strings.hpp"

std::optional<CoverageMode> parseCoverageMode(std::string_view value) {
    std::string normalized(value);
    normalized = toLower(normalized);

    if (normalized == "strict") {
        return CoverageMode::Strict;
    }
    if (normalized == "balanced") {
        return CoverageMode::Balanced;
    }
    if (normalized == "aggressive") {
        return CoverageMode::Aggressive;
    }
    return std::nullopt;
}

const char *coverageModeName(CoverageMode mode) {
    switch (mode) {
    case CoverageMode::Strict:
        return "strict";
    case CoverageMode::Balanced:
        return "balanced";
    case CoverageMode::Aggressive:
        return "aggressive";
    }
    return "balanced";
}
