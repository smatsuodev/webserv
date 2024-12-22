#include "auto_fd.hpp"
#include <unistd.h>
#include "logger.hpp"
#include "string.hpp"

AutoFd::AutoFd(const int fd) : fd_(fd) {}

AutoFd::~AutoFd() {
    if (this->fd_ != -1) {
        LOG_DEBUGF("AutoFd: close %d", this->fd_);
        close(this->fd_);
    }
}

int AutoFd::get() const {
    return this->fd_;
}

int AutoFd::release() {
    const int tmp = this->fd_;
    this->fd_ = -1;
    return tmp;
}
