#include "OracleMode.hpp"

#include "VariableInfo.hpp"

#include <string>

#include "utils/strings.hpp"

namespace {

bool isExplicitLeafType(const InfoType &type) {
    return !type.isContainer() && !type.isRecord() && !type.isOptional() &&
           !type.isPair() && !type.isTuple() && !type.isPointer() &&
           !type.isReference();
}

bool supportsExplicitOracleImpl(const InfoType &type) {
    if (type.original == "void") {
        return false;
    }

    if (type.isPointer() || type.isReference()) {
        return supportsExplicitOracleImpl(type.getUnderlyingType());
    }

    if (type.isOptional() || type.isPair() || type.isTuple()) {
        const auto args = type.getTemplateArguments();
        if (args.empty()) {
            return false;
        }
        for (const auto &arg : args) {
            if (!supportsExplicitOracleImpl(arg)) {
                return false;
            }
        }
        return true;
    }

    if (type.isList()) {
        const auto args = type.getTemplateArguments();
        return args.size() == 1 && isExplicitLeafType(args.front());
    }

    if (type.isMap()) {
        const auto args = type.getTemplateArguments();
        return args.size() == 2 && isExplicitLeafType(args.front()) &&
               isExplicitLeafType(args.back());
    }

    if (type.isRecord()) {
        for (const auto &field : type.getRecordFields()) {
            if (!supportsExplicitOracleImpl(field)) {
                return false;
            }
        }
        return true;
    }

    return true;
}

} // namespace

std::optional<OracleMode> parseOracleMode(std::string_view value) {
    std::string normalized(value);
    normalized = toLower(normalized);

    if (normalized == "mirror") {
        return OracleMode::Mirror;
    }
    if (normalized == "explicit") {
        return OracleMode::Explicit;
    }
    if (normalized == "property") {
        return OracleMode::Property;
    }
    return std::nullopt;
}

const char *oracleModeName(OracleMode mode) {
    switch (mode) {
    case OracleMode::Mirror:
        return "mirror";
    case OracleMode::Explicit:
        return "explicit";
    case OracleMode::Property:
        return "property";
    }
    return "mirror";
}

OracleMode effectiveOracleMode(OracleMode mode) {
    if (mode == OracleMode::Property) {
        return OracleMode::Mirror;
    }
    return mode;
}

bool supportsExplicitOracle(const InfoType &type) {
    return supportsExplicitOracleImpl(type);
}
