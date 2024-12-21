#include "connection.hpp"

Connection::Connection(const int fd):
    clientFd_(fd) {}

int Connection::getFd() const {
    return this->clientFd_.raw();
}
