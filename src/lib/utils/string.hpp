#ifndef SRC_LIB_UTILS_STRING_HPP
#define SRC_LIB_UTILS_STRING_HPP

#include "types/error.hpp"
#include "types/result.hpp"
#include "types/option.hpp"
#include <string>
#include <sstream>
#include <vector>

namespace utils {
    template <class T>
    std::string toString(T value) {
        std::stringstream ss;
        ss << value;

        return ss.str();
    }

    std::string format(const char *fmt, ...);
    std::string join(const std::vector<std::string> &strings, const std::string &delimiter);
    bool startsWith(const std::string &str, const std::string &prefix);
    bool endsWith(const std::string &str, const std::string &suffix);

    // 数字以外を含む場合はエラー (std::stoull とは異なる)
    // 先頭・末尾の空白もエラーとする
    typedef Result<unsigned long, error::AppError> StoulResult;
    StoulResult stoul(const std::string &str);
    std::string trim(const std::string &str);
    std::vector<std::string> split(const std::string &str, char delimiter);

    // std::strnstr は標準ライブラリに含まれず環境によってはビルドできないため, strnstr と同様のものを自作する
    // haystack, needle が NULL の場合は None を返す
    Option<char *> strnstr(const char *haystack, const char *needle, std::size_t len);
}

#endif
