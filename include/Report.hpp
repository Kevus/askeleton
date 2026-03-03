#pragma once

#include <optional>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>

struct ReportEntry {
    std::string kind;
    std::string name;
    std::string qualified_name;
    std::string file;
    unsigned line = 0;
    unsigned column = 0;
    std::string target;
    bool is_class = false;
    std::string status;
    std::string reason;
    std::string detail;
    std::string signature;
    unsigned test_cases = 0;
};

struct ReportMetadata {
    std::string generated_at;
    std::string profile;
    std::string coverage_mode;
    std::string oracle_mode;
    std::optional<uint32_t> seed;
    bool rule_data = false;
    unsigned rule_max_cases = 0;
    std::string framework;
    std::vector<std::string> sources;
};

class Report {
public:
    void setMetadata(const ReportMetadata &meta);
    void addEntry(const ReportEntry &entry);
    bool write(const std::string &path) const;

private:
    nlohmann::json toJson() const;

    ReportMetadata metadata;
    std::vector<ReportEntry> entries;
};
