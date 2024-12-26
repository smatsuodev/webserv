#ifndef SRC_LIB_UTILS_STRING_HPP
#define SRC_LIB_UTILS_STRING_HPP

#include <string>

namespace utils {
    std::string format(const char *fmt, ...);
    bool startsWith(const std::string &str, const std::string &prefix);
    bool endsWith(const std::string &str, const std::string &suffix);
}

#endif
