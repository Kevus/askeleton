#pragma once

#include <map>
#include <optional>
#include <string>

class EquivalentTypesManager {
public:
    static EquivalentTypesManager &getInstance();
    EquivalentTypesManager(const EquivalentTypesManager &) = delete;
    EquivalentTypesManager &operator=(const EquivalentTypesManager &) = delete;

    bool isExcludedType(const std::string &type) const;
    std::optional<std::string> getEquivalentType(const std::string &type) const;

private:
    EquivalentTypesManager(const std::map<std::string, std::string> &);

    std::map<std::string, std::string> excludedTypes;
};
