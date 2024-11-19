#pragma once

#include <nlohmann/json.hpp>

class Config {
public:
    Config(const Config &) = delete;
    Config &operator=(const Config &) = delete;
    Config(Config &&) = delete;
    Config &operator=(Config &&) = delete;

    std::string get(const std::string &key) const;
    const nlohmann::json &operator[](const std::string &key) const;

    static Config &getInstance();

private:
    Config();
    std::string getNestedValue(const std::vector<std::string> &keys) const;

    nlohmann::json configData;
};