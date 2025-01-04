#include "listener.hpp"

#include "utils/fd.hpp"

#include <string>
#include <stdexcept>
#include <arpa/inet.h>
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include <cerrno>
#include <cstring>

Listener::Listener(const Address &listenAddress, const int backlog)
    : serverFd_(Listener::setupSocket(listenAddress, backlog)) {}

Listener::~Listener() {
    LOG_DEBUG("Listener: destruct");
}

int Listener::getFd() const {
    return this->serverFd_.get();
}

Listener::AcceptConnectionResult Listener::acceptConnection() const {
    sockaddr_in sockAddr = {};
    socklen_t sockAddrLen = sizeof(sockAddr);
    const int fd = accept(serverFd_, reinterpret_cast<sockaddr *>(&sockAddr), &sockAddrLen);
    if (fd == -1) {
        LOG_WARNF("failed to accept connection: %s", std::strerror(errno));
        return Err<std::string>("failed to accept connection");
    }
    const Address foreignAddress(inet_ntoa(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

    const Result<void, error::AppError> result = utils::setNonBlocking(fd);
    if (result.isErr()) {
        LOG_ERROR("failed to set non-blocking fd");
        return Err<std::string>("failed to set non-blocking fd");
    }

    LOG_INFOF("connection established from %s (fd: %d)", foreignAddress.toString().c_str(), fd);

    // local address を取得
    sockaddr_in localAddr = {};
    socklen_t localAddrLen = sizeof(localAddr);
    if (getsockname(fd, reinterpret_cast<sockaddr *>(&localAddr), &localAddrLen) == -1) {
        LOG_ERRORF("failed to get local address: %s", std::strerror(errno));
        return Err<std::string>("failed to get local address");
    }
    const Address localAddress(inet_ntoa(localAddr.sin_addr), ntohs(localAddr.sin_port));

    return Ok(new Connection(fd, localAddress, foreignAddress));
}

int Listener::setupSocket(const Address &listenAddress, const int backlog) {
    const int rawFd = socket(AF_INET, SOCK_STREAM, 0);
    if (rawFd == -1) {
        LOG_ERRORF("failed to create socket: %s", std::strerror(errno));
        throw std::runtime_error("failed to create socket");
    }
    LOG_DEBUGF("server socket created (fd: %d)", rawFd);
    AutoFd fd(rawFd);

    const Result<void, error::AppError> setNonBlockingResult = utils::setNonBlocking(rawFd);
    if (setNonBlockingResult.isErr()) {
        LOG_ERROR("failed to set non blocking mode");
        throw std::runtime_error("failed to set non blocking mode");
    }

    const int opt = 1;
    if (setsockopt(rawFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        LOG_ERRORF("failed to set socket options: %s", std::strerror(errno));
        throw std::runtime_error("failed to set socket options");
    }

    sockaddr_in addr = {};
    addr.sin_family = AF_INET;
    // host to network byte order (short)
    addr.sin_port = htons(listenAddress.getPort());
    if (inet_pton(AF_INET, listenAddress.getIp().c_str(), &addr.sin_addr) == -1) {
        LOG_ERRORF("invalid IP address: %s", listenAddress.getIp().c_str());
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

    LOG_DEBUGF("listening on %s (fd: %d)", listenAddress.toString().c_str(), rawFd);

    return fd.release();
}
