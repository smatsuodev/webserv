#include "connection.hpp"
#include "utils/logger.hpp"

Connection::Connection(const int fd, const Address &localAddress, const Address &foreignAddress)
    : clientFd_(fd), localAddress_(localAddress), foreignAddress_(foreignAddress), fdReader_(clientFd_),
      buffer_(fdReader_) {}

Connection::~Connection() {
    LOG_DEBUG("Connection: destruct");
}

int Connection::getFd() const {
    return clientFd_.get();
}

const Address &Connection::getLocalAddress() const {
    return localAddress_;
}

const Address &Connection::getForeignAddress() const {
    return foreignAddress_;
}

ReadBuffer &Connection::getReadBuffer() {
    return buffer_;
}
