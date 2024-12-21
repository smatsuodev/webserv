#ifndef SRC_LIB_UTILS_LOG_HPP
#define SRC_LIB_UTILS_LOG_HPP

#include <string>
#include <sstream>

#define SET_LOG_LEVEL(level) log::Logger::instance().setLevel(level)

#define LOG_DEBUG(msg) log::Logger::instance().log(log::kDebug, msg)
#define LOG_INFO(msg)  log::Logger::instance().log(log::kInfo,  msg)
#define LOG_WARN(msg)  log::Logger::instance().log(log::kWarn,  msg)
#define LOG_ERROR(msg) log::Logger::instance().log(log::kError, msg)

namespace log {
    enum LogLevel {
        kDebug,
        kInfo,
        kWarn,
        kError
    };

    class Logger {
    public:
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
}

#endif
