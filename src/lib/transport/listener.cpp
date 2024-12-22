#include "listener.hpp"
#include <string>
#include <stdexcept>
#include <arpa/inet.h>
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include <cerrno>
#include <cstring>

Listener::Listener(const std::string &ip, const unsigned short port, const int backlog)
    : serverFd_(Listener::setupSocket(ip, port, backlog)) {}

Listener::~Listener() {
    LOG_DEBUG("Listener: destruct");
}

int Listener::getFd() const {
    return this->serverFd_.get();
}

Listener::AcceptConnectionResult Listener::acceptConnection() const {
    sockaddr_in clientAddr = {};
    int clientAddrLen = sizeof(clientAddr);
    const int fd =
        accept(this->getFd(), reinterpret_cast<sockaddr *>(&clientAddr), reinterpret_cast<socklen_t *>(&clientAddrLen));
    if (fd == -1) {
        LOG_WARNF("failed to accept connection: %s", std::strerror(errno));
        return Err<std::string>("failed to accept connection");
    }

    LOG_DEBUGF("connection established from %s:%u (fd: %d)", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port),
               fd);

    return Ok(new Connection(fd));
}

int Listener::setupSocket(const std::string &ip, const unsigned short port, const int backlog) {
    const int rawFd = socket(AF_INET, SOCK_STREAM, 0);
    if (rawFd == -1) {
        LOG_ERRORF("failed to create socket: %s", std::strerror(errno));
        throw std::runtime_error("failed to create socket");
    }
    AutoFd fd(rawFd);

    const int opt = 1;
    if (setsockopt(rawFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        LOG_ERRORF("failed to set socket options: %s", std::strerror(errno));
        throw std::runtime_error("failed to set socket options");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    // host to network byte order (short)
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) == -1) {
        LOG_ERRORF("invalid IP address: %s", ip.c_str());
        throw std::runtime_error("invalid IP address");
    }

    if (bind(rawFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1) {
        LOG_ERRORF("failed to bind socket: %s", std::strerror(errno));
        throw std::runtime_error("failed to bind socket");
    }

    if (listen(rawFd, backlog) == -1) {
        LOG_ERRORF("failed to listen on socket: %s", std::strerror(errno));
        throw std::runtime_error("failed to listen on socket");
    }

    return fd.release();
}
