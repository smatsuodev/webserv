#include "auto_fd.hpp"
#include <unistd.h>
#include "logger.hpp"
#include "string.hpp"

AutoFd::AutoFd() : fd_(-1) {}

AutoFd::AutoFd(const int fd) : fd_(fd) {}

AutoFd::~AutoFd() {
    if (this->fd_ != -1) {
        LOG_DEBUGF("AutoFd: close %d", this->fd_);
        close(this->fd_);
    }
}

int AutoFd::get() const {
    return fd_;
}

int AutoFd::release() {
    const int tmp = fd_;
    fd_ = -1;
    return tmp;
}

void AutoFd::reset(const int fd){
    if (fd_ != -1) {
        close(fd_);
    }
    fd_ = fd;
}

AutoFd::operator int() const{
    return this->get();
}
