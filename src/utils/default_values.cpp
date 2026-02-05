#include "utils/default_values.hpp"

#include <fstream>
#include <sstream>

#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"

using json = nlohmann::json;

namespace {
const json &getDefaultValuesJson() {
    static json values;
    if (values.empty()) {
        const json &config = getConfig();
        std::filesystem::path path = getAskeletonHome() /
                                     config["file"]["data"]["default_values"];
        std::ifstream file(path);
        if (!file.is_open()) {
            return values;
        }
        try {
            file >> values;
        } catch (...) {
            values = json::object();
        }
    }
    return values;
}

std::string normalizeKey(std::string key) {
    replaceAll(key, " ", "_");
    return key;
}

std::optional<std::string> lookupValue(const json &values, const std::string &key) {
    if (!values.contains(key))
        return std::nullopt;
    const auto &val = values.at(key);
    if (val.is_string())
        return val.get<std::string>();
    if (val.is_boolean())
        return val.get<bool>() ? "true" : "false";
    if (val.is_number_integer())
        return std::to_string(val.get<long long>());
    if (val.is_number_unsigned())
        return std::to_string(val.get<unsigned long long>());
    if (val.is_number_float()) {
        std::ostringstream oss;
        oss << val.get<double>();
        return oss.str();
    }
    return std::nullopt;
}
} // namespace

std::optional<std::string> getDefaultValueForType(const InfoType &type) {
    const json &values = getDefaultValuesJson();
    if (values.empty())
        return std::nullopt;

    std::vector<std::string> keys;
    keys.push_back(type.original);
    keys.push_back(normalizeKey(type.original));
    keys.push_back(type.formatted);
    keys.push_back(normalizeKey(type.formatted));
    keys.push_back(removeNamespaceQualifier(type.original));
    keys.push_back(normalizeKey(removeNamespaceQualifier(type.original)));
    keys.push_back(removeTemplateArguments(type.original));
    keys.push_back(normalizeKey(removeTemplateArguments(type.original)));

    for (const auto &key : keys) {
        if (auto value = lookupValue(values, key)) {
            return value;
        }
    }

    return std::nullopt;
}

std::string getZeroValueForType(const InfoType &type) {
    if (containsSubstring(type.original, "string"))
        return "";
    if (type.isContainer())
        return "{}";
    if (type.isRecord())
        return "{}";
    if (type.isEnum())
        return "0";
    if (type.isPointer() || type.isReference())
        return "0";
    if (containsSubstring(type.original, "bool"))
        return "false";
    return "0";
}

std::string formatLiteralForType(const std::string &literal, const InfoType &type) {
    if (containsSubstring(type.original, "string")) {
        std::string escaped = literal;
        replaceAll(escaped, "\\", "\\\\");
        replaceAll(escaped, "\"", "\\\"");
        return "\"" + escaped + "\"";
    }
    if (containsSubstring(type.original, "char") && !containsSubstring(type.original, "char*")) {
        char c = literal.empty() ? '\0' : literal[0];
        if (c == '\0')
            return "'\\0'";
        if (c == '\'')
            return "'\\''";
        return std::string("'") + c + "'";
    }
    if (containsSubstring(type.original, "bool")) {
        return (literal == "true" || literal == "1") ? "true" : "false";
    }
    return literal;
}
