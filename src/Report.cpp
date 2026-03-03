#include "Report.hpp"
#include "utils/system.hpp"

#include <algorithm>
#include <utility>
#include <vector>

using json = nlohmann::json;

namespace {
double computeRatio(std::size_t numerator, std::size_t denominator) {
    if (denominator == 0) {
        return 0.0;
    }
    return static_cast<double>(numerator) / static_cast<double>(denominator);
}

json topCounts(const json &counts, std::size_t limit = 5) {
    std::vector<std::pair<std::string, std::size_t>> items;
    items.reserve(counts.size());
    for (auto it = counts.begin(); it != counts.end(); ++it) {
        items.emplace_back(it.key(), it.value().get<std::size_t>());
    }

    std::sort(items.begin(), items.end(),
              [](const auto &lhs, const auto &rhs) {
                  if (lhs.second != rhs.second) {
                      return lhs.second > rhs.second;
                  }
                  return lhs.first < rhs.first;
              });

    json result = json::array();
    const std::size_t count = std::min(limit, items.size());
    for (std::size_t i = 0; i < count; ++i) {
        result.push_back({{"name", items[i].first}, {"count", items[i].second}});
    }
    return result;
}
} // namespace

void Report::setMetadata(const ReportMetadata &meta) { metadata = meta; }

void Report::addEntry(const ReportEntry &entry) { entries.push_back(entry); }

json Report::toJson() const {
    json root;
    root["generated_at"] = metadata.generated_at;
    root["profile"] = metadata.profile;
    root["coverage_mode"] = metadata.coverage_mode;
    root["oracle_mode"] = metadata.oracle_mode;
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
    json by_reason = json::object();
    json skipped_by_target = json::object();
    json by_file = json::object();

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
        if (!entry.reason.empty())
            by_reason[entry.reason] = by_reason.value(entry.reason, 0) + 1;
        if (!entry.file.empty()) {
            auto &fileSummary = by_file[entry.file];
            if (!fileSummary.is_object()) {
                fileSummary = json::object();
            }
            if (!entry.status.empty()) {
                fileSummary[entry.status] = fileSummary.value(entry.status, 0) + 1;
            }
        }
        if (entry.status == "skipped" && !entry.target.empty()) {
            skipped_by_target[entry.target] =
                skipped_by_target.value(entry.target, 0) + 1;
        }
    }
    root["items"] = items;
    summary["by_status"] = by_status;
    summary["by_kind"] = by_kind;
    summary["by_target"] = by_target;
    summary["by_reason"] = by_reason;
    summary["by_file"] = by_file;
    summary["coverage"] = {
        {"found", entries.size()},
        {"generated", by_status.value("generated", 0)},
        {"skipped", by_status.value("skipped", 0)},
        {"generation_rate", computeRatio(by_status.value("generated", 0), entries.size())},
        {"skip_rate", computeRatio(by_status.value("skipped", 0), entries.size())},
    };
    summary["top_skip_reasons"] = topCounts(by_reason);
    summary["targets_with_most_skips"] = topCounts(skipped_by_target);
    root["summary"] = summary;
    return root;
}

bool Report::write(const std::string &path) const {
    return writeJsonFileAtomically(path, toJson(), 2);
}
