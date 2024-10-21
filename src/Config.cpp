#include "Config.hpp"

#include "utils/strings.hpp"
#include <fstream>

Config &Config::getInstance() {
    static Config instance;
    return instance;
}

void Config::loadConfig(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open config file");
    }

    file >> configData;
}

std::string Config::get(const std::string &key) const {
    std::vector<std::string> keys = key.find('.') == std::string::npos
                                        ? std::vector<std::string>{key}
                                        : split(key, '.');

    return getNestedValue(keys);
}

std::string Config::getNestedValue(const std::vector<std::string> &keys) const {
    const nlohmann::json *currentLevel = &configData;

    for (const auto &key : keys) {
        if (currentLevel->contains(key)) {
            currentLevel = &(*currentLevel)[key];
        } else {
            return {};
        }
    }

    return currentLevel->get<std::string>();
}