#include "Config.hpp"

#include "utils/strings.hpp"
#include "utils/system.hpp"
#include <fstream>

using json = nlohmann::json;

Config &Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() : configData{} {
    std::ifstream file(getAskeletonHome() / "data/configuration.json");
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

const json &Config::operator[](const std::string &key) const {
    return configData[key];
}

std::string Config::getNestedValue(const std::vector<std::string> &keys) const {
    const json *currentLevel = &configData;

    for (const auto &key : keys) {
        if (currentLevel->contains(key)) {
            currentLevel = &(*currentLevel)[key];
        } else {
            return {};
        }
    }

    return currentLevel->get<std::string>();
}