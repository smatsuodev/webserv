#include "auto_close_fd.hpp"
#include <unistd.h>
#include "logger.hpp"
#include "string.hpp"

AutoCloseFd::AutoCloseFd(const int fd) : fd_(fd) {}

AutoCloseFd::~AutoCloseFd() {
    if (this->fd_ != AutoCloseFd::kInvalidFd) {
        LOG_DEBUGF("AutoCloseFd: close %d", this->fd_);
        close(this->fd_);
    }
}

int AutoCloseFd::raw() const {
    return this->fd_;
}

int AutoCloseFd::steal() {
    const int tmp = this->fd_;
    this->fd_ = AutoCloseFd::kInvalidFd;
    return tmp;
}
