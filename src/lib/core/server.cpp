#include "server.hpp"
#include "action.hpp"
#include "event/event_notifier.hpp"
#include "./handler/accept_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
#include <map>

Server::Server(const std::string &ip, const unsigned short port) : ip_(ip), port_(port), listener_(ip, port) {}

Server::~Server() {
    for (std::map<int, Connection *>::iterator it = connections_.begin(); it != connections_.end(); ++it) {
        delete it->second;
    }
    for (std::map<int, IEventHandler *>::iterator it = eventHandlers_.begin(); it != eventHandlers_.end(); ++it) {
        delete it->second;
    }
}

void Server::start() {
    notifier_.registerEvent(Event(listener_.getFd()));
    this->registerEventHandler(listener_.getFd(), new AcceptHandler(listener_));

    LOG_INFOF("server started on port %s:%u", ip_.c_str(), port_);

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
            Context ctx(this->findConnection(ev.getFd()), ev);

            const IEventHandler::InvokeResult result = handler.unwrap()->invoke(ctx);
            if (result.isErr()) {
                this->onHandlerError(ctx, result.unwrapErr());
                continue;
            }

            this->executeActions(result.unwrap());
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

void Server::unregisterEventHandler(const int targetFd) {
    const Option<IEventHandler *> h = this->findEventHandler(targetFd);
    if (h.isNone()) {
        LOG_DEBUG("unregisterEventHandler: handler not found");
        return;
    }
    delete h.unwrap();
    eventHandlers_.erase(targetFd);
    LOG_DEBUGF("event handler removed from fd %d", targetFd);
}

EventNotifier &Server::getEventNotifier() {
    return notifier_;
}

void Server::onHandlerError(const Context &ctx, const error::AppError err) {
    // kIOWouldBlock はリトライするので無視
    if (err == error::kIOWouldBlock) {
        return;
    }

    LOG_WARNF("handler error");

    const Event &ev = ctx.getEvent();
    const Option<Connection *> conn = ctx.getConnection();

    // kIOWouldBlock 以外のエラーはコネクションを閉じる
    if (ev.getFd() != listener_.getFd()) {
        notifier_.unregisterEvent(ev);
        this->unregisterEventHandler(ev.getFd());
        if (conn.isSome()) {
            this->removeConnection(conn.unwrap());
        }
    }
}

void Server::executeActions(std::vector<IAction *> actions) {
    for (std::vector<IAction *>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
        IAction *action = *it;
        action->execute(*this);
        delete action;
    }
}

Option<Connection *> Server::findConnection(const int fd) const {
    const std::map<int, Connection *>::const_iterator it = connections_.find(fd);
    if (it == connections_.end()) {
        return None;
    }
    return Some(it->second);
}

Option<IEventHandler *> Server::findEventHandler(const int fd) {
    const std::map<int, IEventHandler *>::const_iterator it = eventHandlers_.find(fd);
    if (it == eventHandlers_.end()) {
        return None;
    }
    return Some(it->second);
}
