#include "fd.hpp"
#include <fcntl.h>

Result<void, error::AppError> utils::setNonBlocking(const int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        return Err(error::kUnknown);
    }
    return Ok();
}
