#ifndef SRC_LIB_UTILS_FD_HPP
#define SRC_LIB_UTILS_FD_HPP

#include "types/error.hpp"
#include "types/result.hpp"

namespace utils {
    Result<void, error::AppError> setNonBlocking(int fd);
}

#endif
