#include "server_state.hpp"
#include "utils/logger.hpp"

ConnectionRepository::ConnectionRepository() {}

ConnectionRepository::~ConnectionRepository() {
    for (std::map<int, Connection *>::const_iterator it = connections_.begin(); it != connections_.end(); ++it) {
        delete it->second;
    }
}

Option<Connection *> ConnectionRepository::get(const int fd) {
    const std::map<int, Connection *>::const_iterator it = connections_.find(fd);
    if (it == connections_.end()) {
        return None;
    }
    return Some(it->second);
}

void ConnectionRepository::set(const int fd, Connection *conn) {
    if (fd != conn->getFd()) {
        LOG_WARNF("ConnectionRepository::set: fd %d does not match fd %d", fd, conn->getFd());
        // 処理は継続する
    }

    if (connections_[fd]) {
        LOG_DEBUGF("outdated Connection object found for fd %d", fd);
        delete connections_[fd];
    }
    connections_[fd] = conn;
    LOG_DEBUGF("new connection added to server");
}

void ConnectionRepository::remove(const int fd) {
    const Option<Connection *> conn = this->get(fd);
    if (conn.isNone()) {
        LOG_DEBUGF("ConnectionRepository::remove: fd %d does not exist", fd);
        return;
    }

    connections_.erase(fd);
    delete conn.unwrap();

    LOG_DEBUGF("connection removed from server");
}

EventHandlerRepository::EventHandlerRepository() {}

EventHandlerRepository::~EventHandlerRepository() {
    for (std::map<int, IEventHandler *>::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
        delete it->second;
    }
}

Option<IEventHandler *> EventHandlerRepository::get(const int fd) {
    const std::map<int, IEventHandler *>::const_iterator it = handlers_.find(fd);
    if (it == handlers_.end()) {
        return None;
    }
    return Some(it->second);
}

void EventHandlerRepository::set(int fd, IEventHandler *handler) {
    if (handlers_[fd]) {
        LOG_DEBUGF("outdated event handler found for fd %d", fd);
        delete handlers_[fd];
    }

    handlers_[fd] = handler;
    LOG_DEBUGF("event handler added to fd %d", fd);
}

void EventHandlerRepository::remove(const int fd) {
    const Option<IEventHandler *> h = this->get(fd);
    if (h.isNone()) {
        LOG_DEBUG("EventHandlerRepository::remove: handler not found");
        return;
    }

    handlers_.erase(fd);
    delete h.unwrap();

    LOG_DEBUGF("event handler removed from fd %d", fd);
}


EventNotifier &ServerState::getEventNotifier() {
    return notifier_;
}

ConnectionRepository &ServerState::getConnectionRepository() {
    return connRepo_;
}

EventHandlerRepository &ServerState::getEventHandlerRepository() {
    return handlerRepo_;
}
