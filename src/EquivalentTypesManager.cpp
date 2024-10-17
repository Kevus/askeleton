#include "EquivalentTypesManager.hpp"

#include <fstream>
#include <iostream>
#include "constants.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::map<std::string, std::string> readTypesFromFile(
    const std::string &filename = askeleton::files::EQUIVALENT_TYPES) {
    std::ifstream jsonFile(filename);

    if (!jsonFile.is_open()) {
        std::cerr << askeleton::errors::openFileError(filename) << "\n";
        return {};
    }

    std::cout << "Reading from " << filename << "\n";
    json j;

    jsonFile >> j;
    jsonFile.close();

    std::map<std::string, std::string> types;
    for (auto &[key, value] : j.items()) {
        types[key] = value;
    }

    for (const auto &[key, value] : types) {
        std::cout << key << " : " << value << std::endl;
    }

    return types;
}

EquivalentTypesManager::EquivalentTypesManager(
    const std::map<std::string, std::string> &excludedTypes)
    : excludedTypes(excludedTypes) {}

EquivalentTypesManager &EquivalentTypesManager::getInstance() {
    static EquivalentTypesManager instance(readTypesFromFile());
    return instance;
}

bool EquivalentTypesManager::isExcludedType(const std::string &type) const {
    return excludedTypes.find(type) != excludedTypes.end();
}

std::optional<std::string>
EquivalentTypesManager::getEquivalentType(const std::string &type) const {
    auto it = excludedTypes.find(type);
    return it == excludedTypes.end() ? std::nullopt
                                     : std::optional<std::string>(it->second);
}