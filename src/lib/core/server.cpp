#include "server.hpp"
#include <sys/socket.h>
#include "transport/listener.hpp"
#include "utils/string.hpp"
#include "utils/logger.hpp"

#include <cerrno>
#include <string>
#include <sys/epoll.h>
#include <cstring>
#include <fcntl.h>
#include <map>

Server::Server(): epollFd_(-1) {}

Server::~Server() {}

void Server::start(const unsigned short port) {
    /**
     * epoll_create() creates a new epoll(7) instance.
     * Since Linux 2.6.8, the size argument is ignored, but must be greater than zero.
     */
    epollFd_.reset(epoll_create(1));
    if (epollFd_ == -1) {
        LOG_ERRORF("failed to create epoll fd: %s", std::strerror(errno));
        return;
    }
    LOG_DEBUGF("epoll fd created (fd: %d)", epollFd_.get());

    const Listener lsn("0.0.0.0", port);
    Server::setNonBlocking(lsn.getFd());
    this->addToEpoll(lsn.getFd());

    LOG_INFOF("server started on port %u", port);

    epoll_event events[1024];
    std::map<int, Connection *> connections;

    while (true) {
        const int nfds = epoll_wait(epollFd_, events, 1024, -1);
        if (nfds == -1) {
            LOG_ERROR("epoll_wait failed");
            continue;
        }

        for (int i = 0; i < nfds; i++) {
            epoll_event &ev = events[i];
            if (ev.data.fd == lsn.getFd()) {
                LOG_DEBUGF("event arrived on listener (fd: %d)", ev.data.fd);
                const Listener::AcceptConnectionResult result = lsn.acceptConnection();
                if (result.isErr()) {
                    LOG_WARN(result.unwrapErr());
                    continue;
                }
                Connection *conn = result.unwrap();
                connections[conn->getFd()] = conn;
                Server::setNonBlocking(conn->getFd());
                this->addToEpoll(conn->getFd());
            } else {
                LOG_DEBUGF("event arrived on client fd %d", ev.data.fd);
                const Connection *conn = connections[ev.data.fd];
                handleConnection(*conn);
                delete conn;
                connections.erase(ev.data.fd);
            }
        }
    }
}

void Server::handleConnection(const Connection &conn) {
    bufio::Reader reader(conn.getReader());

    const bufio::Reader::ReadAllResult result = reader.readAll();
    if (result.isErr()) {
        LOG_WARN(result.unwrapErr());
        return;
    }
    LOG_DEBUG("finish Reader::readAll");
    const std::string content = result.unwrap();

    if (send(conn.getFd(), content.c_str(), content.size(), 0) == -1) {
        LOG_WARN("failed to send response");
        return;
    }

    LOG_DEBUG("response sent");
}

void Server::addToEpoll(const int fd) const {
    epoll_event ev = {};
    ev.events = EPOLLIN;
    ev.data.fd = fd;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &ev) == -1) {
        LOG_ERROR("failed to add to epoll fd");
        return;
    }

    LOG_DEBUGF("fd %d added to epoll", fd);
}

void Server::setNonBlocking(const int fd){
    const int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
