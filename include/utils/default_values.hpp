#pragma once

#include <optional>
#include <string>

#include "VariableInfo.hpp"

std::optional<std::string> getDefaultValueForType(const InfoType &type);
std::string getZeroValueForType(const InfoType &type);
std::string formatLiteralForType(const std::string &literal, const InfoType &type);
