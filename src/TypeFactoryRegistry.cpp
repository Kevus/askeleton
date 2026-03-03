#include "TypeFactoryRegistry.hpp"

#include <fstream>

#include "Logging.hpp"
#include "utils/strings.hpp"
#include "utils/system.hpp"
#include "utils/templating.hpp"

using json = nlohmann::json;

namespace {
TypeInitStrategy parseStrategy(const std::string &value) {
    std::string v = toLower(value);
    if (v == "factory")
        return TypeInitStrategy::Factory;
    if (v == "dummy")
        return TypeInitStrategy::Dummy;
    if (v == "zeroed")
        return TypeInitStrategy::Zeroed;
    return TypeInitStrategy::Random;
}

std::string normalizeKey(std::string key) {
    replaceAll(key, " ", "_");
    return key;
}

void insertNormalizedFactory(std::map<std::string, TypeFactory> &target,
                             const std::string &key,
                             const TypeFactory &factory) {
    target[key] = factory;
    target[normalizeKey(key)] = factory;
}

std::vector<std::string> makeTypeLookupKeys(const InfoType &type) {
    std::vector<std::string> keys;
    keys.push_back(type.original);
    keys.push_back(normalizeKey(type.original));
    keys.push_back(type.formatted);
    keys.push_back(normalizeKey(type.formatted));
    keys.push_back(removeNamespaceQualifier(type.original));
    keys.push_back(normalizeKey(removeNamespaceQualifier(type.original)));
    keys.push_back(removeTemplateArguments(type.original));
    keys.push_back(normalizeKey(removeTemplateArguments(type.original)));
    return keys;
}
} // namespace

const TypeFactoryRegistry &TypeFactoryRegistry::get() {
    static TypeFactoryRegistry registry;
    return registry;
}

TypeFactoryRegistry::TypeFactoryRegistry() { load(); }

void TypeFactoryRegistry::load() {
    const json &config = getConfig();
    if (!config.contains("file") || !config["file"].contains("data") ||
        !config["file"]["data"].contains("type_factories")) {
        return;
    }

    std::filesystem::path path = getAskeletonHome() /
                                 config["file"]["data"]["type_factories"];
    std::ifstream file(path);
    if (!file.is_open())
        return;

    json data;
    try {
        file >> data;
    } catch (const json::parse_error &e) {
        Logger::instance().warn("Could not parse type factories file " +
                                path.string() + ": " + e.what());
        return;
    }

    if (!data.contains("types") || !data["types"].is_object())
        return;

    for (const auto &[key, value] : data["types"].items()) {
        TypeFactory factory;
        if (value.is_string()) {
            factory.strategy = TypeInitStrategy::Factory;
            factory.expr = value.get<std::string>();
        } else if (value.is_object()) {
            if (value.contains("strategy")) {
                factory.strategy = parseStrategy(value["strategy"].get<std::string>());
            }
            if (value.contains("expr")) {
                factory.expr = value["expr"].get<std::string>();
            }
        }

        insertNormalizedFactory(factories, key, factory);
    }

    if (!data.contains("functions") || !data["functions"].is_object()) {
        return;
    }

    for (const auto &[functionName, functionValue] : data["functions"].items()) {
        if (!functionValue.is_object()) {
            continue;
        }

        const auto *typesNode = &functionValue;
        if (functionValue.contains("types") && functionValue["types"].is_object()) {
            typesNode = &functionValue["types"];
        }

        for (const auto &[typeKey, value] : typesNode->items()) {
            TypeFactory factory;
            if (value.is_string()) {
                factory.strategy = TypeInitStrategy::Factory;
                factory.expr = value.get<std::string>();
            } else if (value.is_object()) {
                if (value.contains("strategy")) {
                    factory.strategy =
                        parseStrategy(value["strategy"].get<std::string>());
                }
                if (value.contains("expr")) {
                    factory.expr = value["expr"].get<std::string>();
                }
            } else {
                continue;
            }

            if (factory.strategy != TypeInitStrategy::Factory || factory.expr.empty()) {
                Logger::instance().warn(
                    "Ignoring function-scoped type factory for " + functionName +
                    " / " + typeKey +
                    " because only explicit factory expressions are supported");
                continue;
            }

            insertNormalizedFactory(functionFactories[functionName], typeKey, factory);
        }
    }
}

std::optional<TypeFactory> TypeFactoryRegistry::find(const InfoType &type,
                                                     const std::string &functionName) const {
    const auto keys = makeTypeLookupKeys(type);

    if (!functionName.empty()) {
        auto functionIt = functionFactories.find(functionName);
        if (functionIt != functionFactories.end()) {
            for (const auto &key : keys) {
                auto it = functionIt->second.find(key);
                if (it != functionIt->second.end()) {
                    return it->second;
                }
            }
        }
    }

    for (const auto &key : keys) {
        auto it = factories.find(key);
        if (it != factories.end())
            return it->second;
    }

    return std::nullopt;
}

std::vector<std::pair<std::string, TypeFactory>>
TypeFactoryRegistry::findFunctionFactories(const InfoType &type) const {
    std::vector<std::pair<std::string, TypeFactory>> matches;
    const auto keys = makeTypeLookupKeys(type);

    for (const auto &[functionName, mapping] : functionFactories) {
        for (const auto &key : keys) {
            auto it = mapping.find(key);
            if (it != mapping.end()) {
                matches.push_back({functionName, it->second});
                break;
            }
        }
    }

    return matches;
}
