#include "server.hpp"
#include "event/event_notifier.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
#include <fcntl.h>
#include <map>

Server::Server() {}

Server::~Server() {}

void Server::start(const unsigned short port) {
    const EventNotifier notifier;

    const Listener lsn("0.0.0.0", port);
    Server::setNonBlocking(lsn.getFd());
    notifier.registerEvent(Event(lsn.getFd()));

    LOG_INFOF("server started on port %u", port);

    std::map<int, Connection *> connections;

    while (true) {
        const EventNotifier::WaitEventsResult waitResult = notifier.waitEvents();
        if (waitResult.isErr()) {
            LOG_ERRORF("EventNotifier::waitEvents failed");
            continue;
        }

        const std::vector<Event> events = waitResult.unwrap();
        for (std::size_t i = 0; i < events.size(); i++) {
            const Event &ev = events[i];
            if (ev.getFd() == lsn.getFd()) {
                LOG_DEBUGF("event arrived on listener (fd: %d)", ev.getFd());
                const Listener::AcceptConnectionResult result = lsn.acceptConnection();
                if (result.isErr()) {
                    LOG_WARN(result.unwrapErr());
                    continue;
                }
                Connection *conn = result.unwrap();
                connections[conn->getFd()] = conn;
                Server::setNonBlocking(conn->getFd());
                notifier.registerEvent(Event(conn->getFd()));
            } else {
                LOG_DEBUGF("event arrived on client fd %d", ev.getFd());
                const Connection *conn = connections[ev.getFd()];
                const Result<HandleConnectionState, error::AppError> result = handleConnection(*conn);
                if (result.isErr() || result.unwrap() == kComplete) {
                    notifier.unregisterEvent(ev);
                    connections.erase(ev.getFd());
                    delete conn;
                }
            }
        }
    }
}

Result<Server::HandleConnectionState, error::AppError> Server::handleConnection(const Connection &conn) {
    const bufio::Reader::ReadAllResult result = conn.getReader().readAll();
    if (result.isErr()) {
        const error::AppError err = result.unwrapErr();
        if (err == error::kIOWouldBlock) {
            return Ok(kSuspend);
        }
        return Err(err);
    }
    LOG_DEBUG("finish Reader::readAll");
    const std::string content = result.unwrap();

    if (send(conn.getFd(), content.c_str(), content.size(), 0) == -1) {
        LOG_WARN("failed to send response");
        return Err(error::kUnknown);
    }

    LOG_DEBUG("response sent");
    return Ok(kComplete);
}

void Server::setNonBlocking(const int fd) {
    const int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
