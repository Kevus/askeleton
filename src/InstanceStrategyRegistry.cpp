#include "InstanceStrategyRegistry.hpp"

#include <fstream>

#include "Logging.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"

using json = nlohmann::json;

namespace {
std::string normalizeKey(std::string key) {
    replaceAll(key, " ", "_");
    return key;
}

void insertNormalized(std::map<std::string, ConfiguredInstanceStrategy> &target,
                      const std::string &key,
                      const ConfiguredInstanceStrategy &strategy) {
    target[key] = strategy;
    target[normalizeKey(key)] = strategy;
}

std::vector<std::string> makeTypeLookupKeys(const InfoType &type) {
    std::vector<std::string> keys;
    keys.push_back(type.original);
    keys.push_back(normalizeKey(type.original));
    keys.push_back(type.formatted);
    keys.push_back(normalizeKey(type.formatted));
    keys.push_back(removeNamespaceQualifier(type.original));
    keys.push_back(normalizeKey(removeNamespaceQualifier(type.original)));
    return keys;
}

std::optional<ConfiguredInstanceStrategy> parseStrategyNode(const json &value) {
    ConfiguredInstanceStrategy strategy;
    if (value.is_string()) {
        strategy.expr = value.get<std::string>();
    } else if (value.is_object() && value.contains("expr") &&
               value["expr"].is_string()) {
        strategy.expr = value["expr"].get<std::string>();
    }

    if (strategy.expr.empty()) {
        return std::nullopt;
    }
    return strategy;
}
} // namespace

const InstanceStrategyRegistry &InstanceStrategyRegistry::get() {
    static InstanceStrategyRegistry registry;
    return registry;
}

InstanceStrategyRegistry::InstanceStrategyRegistry() { load(); }

void InstanceStrategyRegistry::load() {
    const json &config = getConfig();
    if (!config.contains("file") || !config["file"].contains("data") ||
        !config["file"]["data"].contains("instance_strategies")) {
        return;
    }

    std::filesystem::path path = getAskeletonHome() /
                                 config["file"]["data"]["instance_strategies"];
    std::ifstream file(path);
    if (!file.is_open()) {
        return;
    }

    json data;
    try {
        file >> data;
    } catch (const json::parse_error &e) {
        Logger::instance().warn("Could not parse instance strategies file " +
                                path.string() + ": " + e.what());
        return;
    }

    if (data.contains("types") && data["types"].is_object()) {
        for (const auto &[typeKey, value] : data["types"].items()) {
            auto strategy = parseStrategyNode(value);
            if (!strategy.has_value()) {
                continue;
            }
            insertNormalized(typeStrategies, typeKey, *strategy);
        }
    }

    if (data.contains("functions") && data["functions"].is_object()) {
        for (const auto &[functionKey, value] : data["functions"].items()) {
            auto strategy = parseStrategyNode(value);
            if (strategy.has_value()) {
                insertNormalized(functionStrategies, functionKey, *strategy);
                continue;
            }

            if (value.is_object() && value.contains("instance")) {
                strategy = parseStrategyNode(value["instance"]);
                if (strategy.has_value()) {
                    insertNormalized(functionStrategies, functionKey, *strategy);
                }
            }
        }
    }
}

std::optional<ConfiguredInstanceStrategy>
InstanceStrategyRegistry::find(const InfoType &type,
                               const std::string &functionName) const {
    if (!functionName.empty()) {
        std::vector<std::string> functionKeys = {
            functionName,
            normalizeKey(functionName),
        };
        const size_t scopePos = functionName.rfind("::");
        if (scopePos != std::string::npos && scopePos + 2 < functionName.size()) {
            const std::string shortName = functionName.substr(scopePos + 2);
            functionKeys.push_back(shortName);
            functionKeys.push_back(normalizeKey(shortName));
        }

        for (const auto &key : functionKeys) {
            auto it = functionStrategies.find(key);
            if (it != functionStrategies.end()) {
                return it->second;
            }
        }
    }

    for (const auto &key : makeTypeLookupKeys(type)) {
        auto it = typeStrategies.find(key);
        if (it != typeStrategies.end()) {
            return it->second;
        }
    }

    return std::nullopt;
}
