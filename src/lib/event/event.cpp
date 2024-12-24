#include "event.hpp"

Event::Event() : fd_(-1) {}

Event::Event(const int fd) : fd_(fd) {}

Event::Event(const Event &other) : fd_(other.fd_) {}

Event &Event::operator=(const Event &other) {
    if (this != &other) {
        fd_ = other.fd_;
    }
    return *this;
}

int Event::getFd() const {
    return fd_;
}
