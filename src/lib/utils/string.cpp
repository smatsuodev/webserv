#include "string.hpp"
#include "logger.hpp"
#include <cerrno>
#include <vector>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>

std::string utils::format(const char *fmt, ...) {
    // 必要なバッファサイズを計算
    va_list args1;
    va_start(args1, fmt);
    const int size = vsnprintf(NULL, 0, fmt, args1);
    va_end(args1);

    std::vector<char> buffer(size + 1);

    // 実際のフォーマット
    va_list args2;
    va_start(args2, fmt);
    vsnprintf(&buffer[0], size + 1, fmt, args2);
    va_end(args2);

    return std::string(&buffer[0], size);
}

bool utils::startsWith(const std::string &str, const std::string &prefix) {
    return str.find(prefix) == 0;
}

bool utils::endsWith(const std::string &str, const std::string &suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return str.rfind(suffix) == (str.size() - suffix.size());
}

utils::StoulResult utils::stoul(const std::string &str) {
    if (str.empty()) {
        LOG_DEBUGF("utils::stoul: empty string");
        return Err(error::kParseUnknown);
    }

    for (size_t i = 0; i < str.size(); i++) {
        if (!std::isdigit(str[i])) {
            LOG_DEBUGF("utils::stoul: invalid character '%c'", str[i]);
            return Err(error::kParseUnknown);
        }
    }

    errno = 0;
    const unsigned long result = std::strtoul(str.c_str(), NULL, 10);
    if (errno == ERANGE) {
        LOG_DEBUGF("utils::stoul: out of range");
        return Err(error::kParseUnknown);
    }

    return Ok(result);
}
