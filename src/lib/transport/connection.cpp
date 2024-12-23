#include "connection.hpp"
#include "utils/logger.hpp"

Connection::Connection(const int fd)
    : clientFd_(fd), fdReader_(new io::FdReader(clientFd_)), reader_(new bufio::Reader(*fdReader_)) {}

Connection::~Connection() {
    LOG_DEBUG("Connection: destruct");
}

int Connection::getFd() const {
    return clientFd_.get();
}

bufio::Reader &Connection::getReader() const {
    return *reader_;
}
