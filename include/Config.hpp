#pragma once
#include <nlohmann/json.hpp>

class Config
{
public:
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;
	
	void loadConfig(const std::string& path);
	template <typename T> T get(const std::string& key) const;

	static Config& getInstance();

private:
	Config() = default;
	template <typename T> T getNestedValue(const std::vector<std::string>& keys) const;

	nlohmann::json configData;
};
