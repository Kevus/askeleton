#pragma once

#include <map>
#include <string>

struct RunStats {
    unsigned found = 0;
    unsigned generated = 0;
    unsigned skipped = 0;
    std::map<std::string, unsigned> skipped_by_reason;
    std::map<std::string, unsigned> by_kind;
    std::map<std::string, unsigned> by_target;
};
