#include "connection.hpp"
#include "utils/logger.hpp"

Connection::Connection(const int fd) : clientFd_(fd), fdReader_(clientFd_), buffer_(fdReader_) {}

Connection::~Connection() {
    LOG_DEBUG("Connection: destruct");
}

int Connection::getFd() const {
    return clientFd_.get();
}

ReadBuffer &Connection::getReadBuffer() {
    return buffer_;
}
