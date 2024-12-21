// ReSharper disable CppInconsistentNaming
// CLion のバグ? enum に対する命名規約違反の weak warning が出るので無効化

#ifndef SRC_LIB_UTILS_LOG_HPP
#define SRC_LIB_UTILS_LOG_HPP

#include <string>
#include <sstream>

#define SET_LOG_LEVEL(level) Logger::instance().setLevel(level)

#define LOG_DEBUG(msg) Logger::instance().log(Logger::kDebug, msg)
#define LOG_INFO(msg)  Logger::instance().log(Logger::kInfo,  msg)
#define LOG_WARN(msg)  Logger::instance().log(Logger::kWarn,  msg)
#define LOG_ERROR(msg) Logger::instance().log(Logger::kError, msg)

class Logger {
public:
    enum LogLevel {
        kDebug,
        kInfo,
        kWarn,
        kError
    };

    static Logger &instance();
    void setLevel(LogLevel level);
    void log(LogLevel level, const std::string &message) const;

private:
    enum LogColor {
        kColorReset  = 0,
        kColorRed    = 31,
        kColorGreen  = 32,
        kColorYellow = 33,
        kColorBlue   = 34,
        kColorGray   = 37
    };

    LogLevel logLevel_;

    Logger();

    static std::string levelToString(LogLevel level);
    static std::string getTimestamp();
    static std::string colorize(const std::string &text, LogColor color);
};

#endif
