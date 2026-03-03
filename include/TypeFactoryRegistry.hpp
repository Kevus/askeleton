#pragma once

#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "VariableInfo.hpp"

enum class TypeInitStrategy {
    Random,
    Dummy,
    Zeroed,
    Factory,
};

struct TypeFactory {
    TypeInitStrategy strategy = TypeInitStrategy::Random;
    std::string expr;
};

class TypeFactoryRegistry {
public:
    static const TypeFactoryRegistry &get();

    std::optional<TypeFactory> find(const InfoType &type,
                                    const std::string &functionName = "") const;
    std::vector<std::pair<std::string, TypeFactory>>
    findFunctionFactories(const InfoType &type) const;

private:
    TypeFactoryRegistry();
    void load();

    std::map<std::string, TypeFactory> factories;
    std::map<std::string, std::map<std::string, TypeFactory>> functionFactories;
};
