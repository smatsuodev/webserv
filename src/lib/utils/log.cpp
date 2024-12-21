#include "log.hpp"
#include <iostream>
#include <ctime>

using namespace Log;

Logger &Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() :
    logLevel_(DEBUG) {}

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
        case DEBUG:
            return colorize("DEBUG", COLOR_BLUE);
        case INFO:
            return colorize("INFO", COLOR_GREEN);
        case WARN:
            return colorize("WARN", COLOR_YELLOW);
        case ERROR:
            return colorize("ERROR", COLOR_RED);
        default:
            return colorize("UNKNOWN", COLOR_GRAY);
    }
}

std::string Logger::getTimestamp() {
    const std::time_t nowTime = std::time(NULL);
    const tm *nowLocal = std::localtime(&nowTime);

    char buffer[20];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", nowLocal);
    return std::string(buffer);
}

std::string Logger::colorize(const std::string &text, const LogColor color) {
    std::ostringstream oss;
    oss << "\033[" << color << "m" << text << "\033[0m";
    return oss.str();
}
