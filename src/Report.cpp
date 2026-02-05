#include "Report.hpp"

#include <fstream>

using json = nlohmann::json;

void Report::setMetadata(const ReportMetadata &meta) { metadata = meta; }

void Report::addEntry(const ReportEntry &entry) { entries.push_back(entry); }

json Report::toJson() const {
    json root;
    root["generated_at"] = metadata.generated_at;
    root["profile"] = metadata.profile;
    if (metadata.seed.has_value()) {
        root["seed"] = metadata.seed.value();
    }
    root["rule_data"] = metadata.rule_data;
    root["rule_max_cases"] = metadata.rule_max_cases;
    root["framework"] = metadata.framework;
    root["sources"] = metadata.sources;

    json items = json::array();
    for (const auto &entry : entries) {
        json e;
        e["kind"] = entry.kind;
        e["name"] = entry.name;
        e["qualified_name"] = entry.qualified_name;
        e["file"] = entry.file;
        e["line"] = entry.line;
        e["column"] = entry.column;
        e["target"] = entry.target;
        e["is_class"] = entry.is_class;
        e["status"] = entry.status;
        if (!entry.reason.empty())
            e["reason"] = entry.reason;
        if (!entry.detail.empty())
            e["detail"] = entry.detail;
        if (entry.test_cases > 0)
            e["test_cases"] = entry.test_cases;
        items.push_back(e);
    }
    root["items"] = items;
    return root;
}

void Report::write(const std::string &path) const {
    std::ofstream out(path);
    if (!out.is_open())
        return;
    out << toJson().dump(2) << "\n";
}
