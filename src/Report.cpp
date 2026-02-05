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
    json summary;
    json by_status = json::object();
    json by_kind = json::object();
    json by_target = json::object();

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
        if (!entry.signature.empty())
            e["signature"] = entry.signature;
        if (entry.test_cases > 0)
            e["test_cases"] = entry.test_cases;
        items.push_back(e);

        if (!entry.status.empty())
            by_status[entry.status] = by_status.value(entry.status, 0) + 1;
        if (!entry.kind.empty())
            by_kind[entry.kind] = by_kind.value(entry.kind, 0) + 1;
        if (!entry.target.empty())
            by_target[entry.target] = by_target.value(entry.target, 0) + 1;
    }
    root["items"] = items;
    summary["by_status"] = by_status;
    summary["by_kind"] = by_kind;
    summary["by_target"] = by_target;
    root["summary"] = summary;
    return root;
}

void Report::write(const std::string &path) const {
    std::ofstream out(path);
    if (!out.is_open())
        return;
    out << toJson().dump(2) << "\n";
}
