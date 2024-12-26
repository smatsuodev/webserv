#ifndef SRC_LIB_UTILS_STRING_HPP
#define SRC_LIB_UTILS_STRING_HPP

#include "types/error.hpp"
#include "types/result.hpp"
#include <string>

namespace utils {
    std::string format(const char *fmt, ...);
    bool startsWith(const std::string &str, const std::string &prefix);
    bool endsWith(const std::string &str, const std::string &suffix);
    // 数字以外を含む場合はエラー (std::stoull とは異なる)
    // 先頭・末尾の空白もエラーとする
    typedef Result<unsigned long, error::AppError> StoulResult;
    StoulResult stoul(const std::string &str);
}

#endif
