#include "connection.hpp"
#include "utils/logger.hpp"

Connection::Connection(const int fd, const Endpoint &listenerEndpoint, const Endpoint &clientEndpoint)
    : clientFd_(fd), listenerEndpoint_(listenerEndpoint), clientEndpoint_(clientEndpoint), fdReader_(clientFd_),
      buffer_(fdReader_) {}

Connection::~Connection() {
    LOG_DEBUG("Connection: destruct");
}

int Connection::getFd() const {
    return clientFd_.get();
}

const Endpoint &Connection::getListenerEndpoint() const {
    return listenerEndpoint_;
}

const Endpoint &Connection::getClientEndpoint() const {
    return clientEndpoint_;
}

ReadBuffer &Connection::getReadBuffer() {
    return buffer_;
}
