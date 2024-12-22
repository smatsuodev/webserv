#include "connection.hpp"
#include "utils/logger.hpp"

Connection::Connection(const int fd) : clientFd_(fd), reader_(new io::FdReader(clientFd_)) {}

Connection::~Connection() {
    LOG_DEBUG("Connection: destruct");
}

int Connection::getFd() const {
    return clientFd_.get();
}

io::IReader &Connection::getReader() const {
    return *reader_;
}
