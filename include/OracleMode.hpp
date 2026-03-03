#pragma once

#include <optional>
#include <string_view>

struct InfoType;

enum class OracleMode {
    Mirror,
    Explicit,
    Property,
};

std::optional<OracleMode> parseOracleMode(std::string_view value);
const char *oracleModeName(OracleMode mode);
OracleMode effectiveOracleMode(OracleMode mode);
bool supportsExplicitOracle(const InfoType &type);
