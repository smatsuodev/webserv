#include "server_state.hpp"
#include "utils/logger.hpp"
#include "utils/ref.hpp"

ConnectionRepository::ConnectionRepository() {}

ConnectionRepository::~ConnectionRepository() {
    for (std::map<int, Connection *>::const_iterator it = connections_.begin(); it != connections_.end(); ++it) {
        delete it->second;
    }
}

Option<Ref<Connection> > ConnectionRepository::get(const int fd) {
    const std::map<int, Connection *>::const_iterator it = connections_.find(fd);
    if (it == connections_.end()) {
        return None;
    }
    return Some(Ref<Connection>(*it->second));
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
    const std::map<int, Connection *>::const_iterator it = connections_.find(fd);
    if (it == connections_.end()) {
        LOG_DEBUGF("ConnectionRepository::remove: fd %d does not exist", fd);
        return;
    }

    delete it->second;
    connections_.erase(fd);

    LOG_DEBUGF("connection removed from server");
}

EventHandlerRepository::EventHandlerRepository() {}

EventHandlerRepository::~EventHandlerRepository() {
    for (std::map<Key, IEventHandler *>::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
        delete it->second;
    }
}

Option<Ref<IEventHandler> > EventHandlerRepository::get(const int fd, const Event::EventType type) {
    const Key key(fd, type);
    const std::map<Key, IEventHandler *>::const_iterator it = handlers_.find(key);
    if (it == handlers_.end()) {
        return None;
    }
    return Some(Ref<IEventHandler>(*it->second));
}

void EventHandlerRepository::set(const int fd, Event::EventType type, IEventHandler *handler) {
    const Key key(fd, type);
    if (handlers_[key]) {
        LOG_DEBUGF("outdated event handler found for fd %d", fd);
        delete handlers_[key];
    }

    handlers_[key] = handler;
    LOG_DEBUGF("event handler added to fd %d (type: %d)", fd, type);
}

void EventHandlerRepository::remove(const int fd, Event::EventType type) {
    const Key key(fd, type);
    const std::map<Key, IEventHandler *>::const_iterator it = handlers_.find(key);
    if (it == handlers_.end()) {
        LOG_DEBUG("EventHandlerRepository::remove: handler not found");
        return;
    }

    delete it->second;
    handlers_.erase(key);

    LOG_DEBUGF("event handler removed from fd %d (type: %d)", fd, type);
}

Option<CgiProcessRepository::Data> CgiProcessRepository::get(const pid_t pid) {
    const std::map<int, Data>::iterator it = pidToData_.find(pid);
    if (it == pidToData_.end()) {
        return None;
    }
    return Some(it->second);
}

void CgiProcessRepository::set(const pid_t pid, const Data data) {
    // TODO: 上書きしてよいのか?
    pidToData_[pid] = data;
}

void CgiProcessRepository::remove(const pid_t pid) {
    pidToData_.erase(pid);
}

ServerState::ServerState() {
    // self-pipe の読み端を監視対象にする
    reaper_.attachToEventNotifier(&getEventNotifier());
}

IEventNotifier &ServerState::getEventNotifier() {
    return notifier_;
}

ConnectionRepository &ServerState::getConnectionRepository() {
    return connRepo_;
}

EventHandlerRepository &ServerState::getEventHandlerRepository() {
    return handlerRepo_;
}

CgiProcessRepository &ServerState::getCgiProcessRepository() {
    return cgiProcessRepo_;
}

ChildReaper &ServerState::getChildReaper() {
    return reaper_;
}
