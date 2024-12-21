#include "listener.hpp"
#include "auto_close_fd.hpp"
#include <string>
#include <stdexcept>
#include <arpa/inet.h>
#include <cstdio>

Listener::Listener(const std::string &ip, const unsigned short port, const int backlog) :
    serverFd_(Listener::setupSocket(ip, port, backlog)) {}

int Listener::getFd() const {
    return this->serverFd_.raw();
}

Connection *Listener::acceptConnection() const {
    sockaddr_in clientAddr = {};
    int clientAddrLen = sizeof(clientAddr);
    const int fd = accept(
        this->getFd(),
        reinterpret_cast<sockaddr *>(&clientAddr),
        reinterpret_cast<socklen_t *>(&clientAddrLen)
        );
    if (fd == -1) {
        std::perror("accept");
        return NULL;
    }

    return new Connection(fd);
}

int Listener::setupSocket(const std::string &ip, const unsigned short port, const int backlog) {
    const int rawFd = socket(AF_INET, SOCK_STREAM, 0);
    if (rawFd == -1) {
        throw std::runtime_error("Failed to create socket");
    }
    AutoCloseFd fd(rawFd);

    const int opt = 1;
    if (setsockopt(rawFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        throw std::runtime_error("Failed to set socket options");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    // host to network byte order (short)
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip.c_str(), &addr.sin_addr) == -1) {
        throw std::runtime_error("Invalid IP address");
    }

    if (bind(rawFd, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) == -1) {
        throw std::runtime_error("Failed to bind socket");
    }

    if (listen(rawFd, backlog) == -1) {
        throw std::runtime_error("Failed to listen on socket");
    }

    return fd.steal();
}
