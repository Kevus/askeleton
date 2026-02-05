#include "Logging.hpp"

#include <filesystem>

#include "color.h"
#include "llvm/Support/raw_ostream.h"

namespace {
std::string formatPathLabel(const std::string &path) {
    std::filesystem::path p(path);
    if (p.has_filename())
        return p.filename().string();
    return path;
}
} // namespace

Logger &Logger::instance() {
    static Logger logger;
    return logger;
}

void Logger::setLevel(LogLevel level) { level_ = level; }
LogLevel Logger::level() const { return level_; }

void Logger::info(const std::string &message) const {
    if (level_ >= LogLevel::Normal)
        llvm::outs() << message << "\n";
}

void Logger::warn(const std::string &message) const {
    if (level_ >= LogLevel::Normal)
        llvm::outs() << ANSI_YELLOW << "WARNING: " << message << ANSI_RESET << "\n";
}

void Logger::verbose(const std::string &message) const {
    if (level_ >= LogLevel::Verbose)
        llvm::outs() << message << "\n";
}

void Logger::debug(const std::string &message) const {
    if (level_ >= LogLevel::Debug)
        llvm::outs() << message << "\n";
}

void Logger::recordMissingSource(const std::string &filePath) {
    missingSourceFiles_.insert(filePath);
}

void Logger::recordMissingHeader(const std::string &filePath) {
    missingHeaderFiles_.insert(filePath);
}

void Logger::recordFileSeen(const std::string &filePath) {
    std::filesystem::path p(filePath);
    if (p.is_relative())
        p = std::filesystem::absolute(p);
    std::string normalized = p.lexically_normal().string();

    if (filesSeen_.insert(normalized).second && level_ >= LogLevel::Normal) {
        llvm::outs() << ANSI_BLUE << "Processing file: "
                     << formatPathLabel(normalized) << ANSI_RESET << "\n";
    }
}

const std::set<std::string> &Logger::missingSourceFiles() const {
    return missingSourceFiles_;
}

const std::set<std::string> &Logger::missingHeaderFiles() const {
    return missingHeaderFiles_;
}

const std::set<std::string> &Logger::filesSeen() const { return filesSeen_; }

void Logger::printWarningsSummary() const {
    if (level_ < LogLevel::Normal)
        return;

    if (!missingSourceFiles_.empty()) {
        llvm::outs() << ANSI_YELLOW << "Missing source files (" << missingSourceFiles_.size()
                     << "):" << ANSI_RESET << "\n";
        for (const auto &path : missingSourceFiles_) {
            llvm::outs() << "  - " << path << "\n";
        }
    }

    if (!missingHeaderFiles_.empty()) {
        llvm::outs() << ANSI_YELLOW << "Missing header files (" << missingHeaderFiles_.size()
                     << "):" << ANSI_RESET << "\n";
        for (const auto &path : missingHeaderFiles_) {
            llvm::outs() << "  - " << path << "\n";
        }
    }
}
