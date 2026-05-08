#pragma once
#include <string>

namespace askeleton {
const std::string ASKELETON_VARNAME = "ASKELETON_HOME";
const std::string ASKELETON_VERSION = "1.0.1";

namespace errors {
const auto openFileError = [](const std::string &filename) {
    return "File " + filename +
           " couldn't be opened. Check permissions and the route.";
};
} // namespace errors

} // namespace askeleton
