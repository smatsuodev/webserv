#include "fd.hpp"
#include "logger.hpp"
#include <fcntl.h>

Result<void, error::AppError> utils::setNonBlocking(const int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        LOG_WARNF("failed to set O_NONBLOCK to fd %d", fd);
        return Err(error::kUnknown);
    }
    return Ok();
}
