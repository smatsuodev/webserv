#ifndef SRC_LIB_UTILS_LOG_HPP
#define SRC_LIB_UTILS_LOG_HPP

#include <string>
#include <sstream>

#define SET_LOG_LEVEL(level) Log::Logger::instance().setLevel(level)

#define LOG_DEBUG(msg) Log::Logger::instance().log(Log::DEBUG, msg)
#define LOG_INFO(msg)  Log::Logger::instance().log(Log::INFO,  msg)
#define LOG_WARN(msg)  Log::Logger::instance().log(Log::WARN,  msg)
#define LOG_ERROR(msg) Log::Logger::instance().log(Log::ERROR, msg)

namespace Log {
    enum LogLevel {
        DEBUG,
        INFO,
        WARN,
        ERROR
    };

    class Logger {
    public:
        static Logger &instance();
        void setLevel(LogLevel level);
        void log(LogLevel level, const std::string &message) const;

    private:
        enum LogColor {
            COLOR_RESET  = 0,
            COLOR_RED    = 31,
            COLOR_GREEN  = 32,
            COLOR_YELLOW = 33,
            COLOR_BLUE   = 34,
            COLOR_GRAY   = 37
        };

        LogLevel logLevel_;

        Logger();

        static std::string levelToString(LogLevel level);
        static std::string getTimestamp();
        static std::string colorize(const std::string &text, LogColor color);
    };
}

#endif
