#include "connection.hpp"
#include "utils/logger.hpp"

Connection::Connection(const int fd):
    clientFd_(fd) {}

Connection::~Connection() {
    LOG_DEBUG("Connection: destruct");
}

int Connection::getFd() const {
    return this->clientFd_.raw();
}
