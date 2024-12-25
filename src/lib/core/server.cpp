#include "server.hpp"

#include "action.hpp"
#include "event/event_notifier.hpp"
#include "event/handler/accept_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
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
    this->registerEventHandler(lsn.getFd(), new AcceptHandler(lsn));

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
            LOG_DEBUGF("event arrived for fd %d", ev.getFd());

            Option<IEventHandler *> handler = this->findEventHandler(ev.getFd());
            if (handler.isNone()) {
                LOG_DEBUGF("event handler for fd %d is not registered", ev.getFd());
                continue;
            }
            Option<Connection *> connection = this->findConnection(ev.getFd());
            Context ctx(connection, ev);

            const IEventHandler::InvokeResult result = handler.unwrap()->invoke(ctx);
            if (result.isErr()) {
                const error::AppError err = result.unwrapErr();
                if (err == error::kIOWouldBlock) {
                    continue;
                }

                LOG_WARNF("handler error");
                // kIOWouldBlock 以外のエラーはコネクションを閉じる
                if (ev.getFd() != lsn.getFd()) {
                    notifier_.unregisterEvent(ev);
                    if (connection.isSome()) {
                        this->removeConnection(connection.unwrap());
                    }
                }
                continue;
            }

            const std::vector<IAction *> actions = result.unwrap();
            for (std::vector<IAction *>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
                IAction *action = *it;
                action->execute(*this);
                delete action;
            }
        }
    }
}

void Server::addConnection(Connection *conn) {
    if (connections_[conn->getFd()]) {
        LOG_DEBUGF("outdated Connection object found for fd %d", conn->getFd());
        delete connections_[conn->getFd()];
    }
    connections_[conn->getFd()] = conn;
    LOG_DEBUGF("new connection added to server");
}

void Server::removeConnection(const Connection *conn) {
    connections_.erase(conn->getFd());
    delete conn;
    LOG_DEBUGF("connection removed from server");
}

void Server::registerEventHandler(const int targetFd, IEventHandler *handler) {
    if (eventHandlers_[targetFd]) {
        LOG_DEBUGF("outdated event handler found for fd %d", targetFd);
        delete eventHandlers_[targetFd];
    }
    eventHandlers_[targetFd] = handler;
    LOG_DEBUGF("event handler added to fd %d", targetFd);
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
