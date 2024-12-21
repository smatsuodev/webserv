#include "auto_close_fd.hpp"
#include <unistd.h>
#include "utils/log.hpp"
#include "utils/string.hpp"

AutoCloseFd::AutoCloseFd(const int fd):
    fd_(fd) {}

AutoCloseFd::~AutoCloseFd() {
    if (this->fd_ != AutoCloseFd::kInvalidFd) {
        LOG_DEBUG(utils::format("AutoCloseFd: close %d", this->fd_));
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
