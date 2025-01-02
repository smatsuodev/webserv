#ifndef SRC_LIB_CORE_SERVER_STATE_HPP
#define SRC_LIB_CORE_SERVER_STATE_HPP

#include "event/event_notifier.hpp"
#include "event/event_handler.hpp"
#include "transport/connection.hpp"
#include "utils/types/option.hpp"
#include <map>

// TODO: 共通化するべき?

class ConnectionRepository : public NonCopyable {
public:
    ConnectionRepository();
    ~ConnectionRepository();

    Option<Ref<Connection> > get(int fd);
    /**
     * Connection#getFd があるので、fd はなくてもいい
     * インターフェースの統一のためにある
     */
    void set(int fd, Connection *conn);
    void remove(int fd);

private:
    std::map<int, Connection *> connections_;
};

class EventHandlerRepository : public NonCopyable {
public:
    EventHandlerRepository();
    ~EventHandlerRepository();

    Option<Ref<IEventHandler> > get(int fd);
    void set(int fd, IEventHandler *handler);
    void remove(int fd);

private:
    std::map<int, IEventHandler *> handlers_;
};

class ServerState {
public:
    IEventNotifier &getEventNotifier();
    ConnectionRepository &getConnectionRepository();
    EventHandlerRepository &getEventHandlerRepository();

private:
    // EventNotifier はあんまり state っぽくない
#if defined(__linux__)
    EpollEventNotifier notifier_;
#else
    PollEventNotifier notifier_;
#endif

    ConnectionRepository connRepo_;
    EventHandlerRepository handlerRepo_;
};

#endif
