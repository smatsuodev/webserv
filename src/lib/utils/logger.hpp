#ifndef SRC_LIB_UTILS_LOG_HPP
#define SRC_LIB_UTILS_LOG_HPP

#include <string>
#include <sstream>
#include "utils/string.hpp" // LOG_*F で必要

#define SET_LOG_LEVEL(level) ::Logger::instance().setLevel(level)

#define LOG_DEBUG(msg) ::Logger::instance().log(::Logger::kDebug, msg)
#define LOG_INFO(msg) ::Logger::instance().log(::Logger::kInfo, msg)
#define LOG_WARN(msg) ::Logger::instance().log(::Logger::kWarn, msg)
#define LOG_ERROR(msg) ::Logger::instance().log(::Logger::kError, msg)

#define LOG_DEBUGF(...) ::Logger::instance().log(::Logger::kDebug, utils::format(__VA_ARGS__))
#define LOG_INFOF(...) ::Logger::instance().log(::Logger::kInfo, utils::format(__VA_ARGS__))
#define LOG_WARNF(...) ::Logger::instance().log(::Logger::kWarn, utils::format(__VA_ARGS__))
#define LOG_ERRORF(...) ::Logger::instance().log(::Logger::kError, utils::format(__VA_ARGS__))

class Logger {
public:
    enum LogLevel { kDebug, kInfo, kWarn, kError };

    static Logger &instance();
    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string &message) const;

private:
    enum LogColor {
        kColorReset = 0,
        kColorRed = 31,
        kColorGreen = 32,
        kColorYellow = 33,
        kColorBlue = 34,
        kColorGray = 37
    };

    LogLevel logLevel_;

    Logger();

    static std::string levelToString(LogLevel level);
    static std::string getTimestamp();
    static std::string colorize(const std::string &text, LogColor color);
};

#endif
