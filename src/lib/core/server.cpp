#include "server.hpp"
#include "event/event_notifier.hpp"
#include "event/handler/accept_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
#include <fcntl.h>
#include <map>

Server::Server() {}

Server::~Server() {
    for (std::map<int, Connection *>::iterator it = connections_.begin(); it != connections_.end(); ++it) {
        delete it->second;
    }
    for (std::map<int, IEventHandler *>::iterator it = eventHandlers_.begin(); it != eventHandlers_.end(); ++it) {
        delete it->second;
    }
}

void Server::start(const unsigned short port) {
    Listener lsn("0.0.0.0", port);
    notifier_.registerEvent(Event(lsn.getFd()));
    this->registerEventHandler(lsn.getFd(), new AcceptHandler(notifier_, lsn));

    LOG_INFOF("server started on port %u", port);

    while (true) {
        const EventNotifier::WaitEventsResult waitResult = notifier_.waitEvents();
        if (waitResult.isErr()) {
            LOG_ERRORF("EventNotifier::waitEvents failed");
            continue;
        }

        const std::vector<Event> events = waitResult.unwrap();
        for (std::size_t i = 0; i < events.size(); i++) {
            const Event &ev = events[i];
            Option<IEventHandler *> maybeHandler = this->findEventHandler(ev.getFd());
            if (maybeHandler.isNone()) {
                LOG_DEBUGF("event handler for fd %d is not registered", ev.getFd());
                continue;
            }
            IEventHandler *handler = maybeHandler.unwrap();
            Option<Connection *> maybeConnection = this->findConnection(ev.getFd());
            Context ctx(maybeConnection, ev);

            const IEventHandler::InvokeResult result = handler->invoke(ctx);
            if (ev.getFd() != lsn.getFd()) {
                // listening socket 以外の後処理
                if (result.isOk() || result.unwrapErr() != error::kIOWouldBlock) {
                    // kIOWouldBlock 以外は connection を削除する
                    notifier_.unregisterEvent(ev);
                    connections_.erase(ev.getFd());
                    delete maybeConnection.unwrap();
                }
            }
        }
    }
}

void Server::addConnection(Connection *conn) {
    LOG_DEBUGF("new connection added to server");
    connections_[conn->getFd()] = conn;
}

void Server::removeConnection(const Connection *conn) {
    LOG_DEBUGF("connection removed from server");
    connections_.erase(conn->getFd());
    delete conn;
}

void Server::registerEventHandler(const int targetFd, IEventHandler *handler) {
    LOG_DEBUGF("event handler added to fd %d", targetFd);
    eventHandlers_[targetFd] = handler;
}

EventNotifier &Server::getEventNotifier() {
    return notifier_;
}

Option<Connection *> Server::findConnection(const int fd) const {
    const std::map<int, Connection *>::const_iterator it = connections_.find(fd);
    if (it == connections_.end()) {
        return None;
    }
    return Some(it->second);
}

Option<IEventHandler *> Server::findEventHandler(int fd) {
    const std::map<int, IEventHandler *>::const_iterator it = eventHandlers_.find(fd);
    if (it == eventHandlers_.end()) {
        return None;
    }
    return Some(it->second);
}
