#include "log.hpp"
#include <iostream>
#include <ctime>

using namespace log;

Logger &Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() :
    logLevel_(kDebug) {}

void Logger::setLevel(const LogLevel level) {
    this->logLevel_ = level;
}

void Logger::log(const LogLevel level, const std::string &message) const {
    if (level < this->logLevel_) {
        return;
    }

    const std::string levelStr = Logger::levelToString(level);
    const std::string timestamp = Logger::getTimestamp();

    std::ostringstream logEntry;
    logEntry << "[" << timestamp << "] [" << levelStr << "] " << message << std::endl;

    std::cout << logEntry.str();
}

std::string Logger::levelToString(const LogLevel level) {
    switch (level) {
        case kDebug:
            return colorize("DEBUG", kColorBlue);
        case kInfo:
            return colorize("INFO", kColorGreen);
        case kWarn:
            return colorize("WARN", kColorYellow);
        case kError:
            return colorize("ERROR", kColorRed);
        default:
            return colorize("UNKNOWN", kColorGray);
    }
}

std::string Logger::getTimestamp() {
    const std::time_t nowTime = std::time(NULL);
    const tm *nowLocal = std::localtime(&nowTime);

    char buffer[32];
    std::strftime(buffer, sizeof(buffer), "%Y/%m/%d %H:%M:%S%z", nowLocal);
    return std::string(buffer);
}

std::string Logger::colorize(const std::string &text, const LogColor color) {
    std::ostringstream oss;
    oss << "\033[" << color << "m" << text << "\033[0m";
    return oss.str();
}
