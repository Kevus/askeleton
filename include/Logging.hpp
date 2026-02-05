#pragma once

#include <set>
#include <string>

enum class LogLevel { Quiet = 0, Normal = 1, Verbose = 2, Debug = 3 };

class Logger {
public:
    static Logger &instance();

    void setLevel(LogLevel level);
    LogLevel level() const;

    void info(const std::string &message) const;
    void warn(const std::string &message) const;
    void verbose(const std::string &message) const;
    void debug(const std::string &message) const;

    void recordMissingSource(const std::string &filePath);
    void recordMissingHeader(const std::string &filePath);
    void recordFileSeen(const std::string &filePath);

    const std::set<std::string> &missingSourceFiles() const;
    const std::set<std::string> &missingHeaderFiles() const;
    const std::set<std::string> &filesSeen() const;

    void printWarningsSummary() const;

private:
    Logger() = default;

    LogLevel level_ = LogLevel::Normal;
    std::set<std::string> missingSourceFiles_;
    std::set<std::string> missingHeaderFiles_;
    std::set<std::string> filesSeen_;
};
