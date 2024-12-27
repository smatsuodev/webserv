#include "event.hpp"

Event::Event() : fd_(-1), typeFlags_(0) {}

Event::Event(const int fd, const uint32_t typeFlags) : fd_(fd), typeFlags_(typeFlags) {}

Event::Event(const Event &other) : fd_(other.fd_), typeFlags_(other.typeFlags_) {}

Event &Event::operator=(const Event &other) {
    if (this != &other) {
        fd_ = other.fd_;
        typeFlags_ = other.typeFlags_;
    }
    return *this;
}

int Event::getFd() const {
    return fd_;
}

uint32_t Event::getTypeFlags() const {
    return typeFlags_;
}
