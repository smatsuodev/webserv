#include "string.hpp"
#include "logger.hpp"
#include <cerrno>
#include <vector>
#include <cstdlib>
#include <cstdarg>
#include <cstdio>
#include <iterator>

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

std::string utils::join(const std::vector<std::string> &strings, const std::string &delimiter) {
    if (strings.empty()) return "";

    std::ostringstream oss;
    std::copy(strings.begin(), strings.end() - 1, std::ostream_iterator<std::string>(oss, delimiter.c_str()));
    oss << strings.back();
    return oss.str();
}

bool utils::startsWith(const std::string &str, const std::string &prefix) {
    return str.find(prefix) == 0;
}

bool utils::endsWith(const std::string &str, const std::string &suffix) {
    if (str.size() < suffix.size()) {
        return false;
    }
    return str.rfind(suffix) == str.size() - suffix.size();
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


    const unsigned long result = std::strtoul(str.c_str(), NULL, 10);
    if (errno == ERANGE) {
        LOG_DEBUGF("utils::stoul: out of range");
        return Err(error::kParseUnknown);
    }

    return Ok(result);
}

// TODO: コピーを避ける
Option<char *> utils::strnstr(const char *haystack, const char *needle, const std::size_t len) {
    if (haystack == NULL || needle == NULL) {
        return None;
    }

    std::string haystackStr(haystack, len);
    const std::size_t pos = haystackStr.find(needle);
    if (pos == std::string::npos) {
        return None;
    }
    return Some(const_cast<char *>(haystack + pos));
}

std::string utils::trim(const std::string &str) {
    size_t start = 0;
    while (start < str.length() && std::isspace(str[start]))
        start++;
    size_t end = str.length();
    while (end > start && std::isspace(str[end - 1]))
        end--;
    return str.substr(start, end - start);
}

std::vector<std::string> utils::split(const std::string &str, const char delimiter) {
    std::vector<std::string> result;
    std::string::size_type start = 0;

    std::string::size_type end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));

    return result;
}
