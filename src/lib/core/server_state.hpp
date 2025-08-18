#ifndef SRC_LIB_CORE_SERVER_STATE_HPP
#define SRC_LIB_CORE_SERVER_STATE_HPP

#include "child_reaper.hpp"
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

    // get, set, remove が受け取る Event::EventType は、どれか 1 bit が立ったフラグのみを想定している (和はダメ)
    Option<Ref<IEventHandler> > get(int fd, Event::EventType type);
    void set(int fd, Event::EventType type, IEventHandler *handler);
    void remove(int fd, Event::EventType type);

private:
    typedef std::pair<int, Event::EventType> Key;
    std::map<Key, IEventHandler *> handlers_;
};

class ServerState {
public:
    ServerState();

    IEventNotifier &getEventNotifier();
    ConnectionRepository &getConnectionRepository();
    EventHandlerRepository &getEventHandlerRepository();
    ChildReaper &getChildReaper();

private:
    // EventNotifier はあんまり state っぽくない
#if defined(__linux__) && USE_EPOLL
    EpollEventNotifier notifier_;
#else
    PollEventNotifier notifier_;
#endif

    ChildReaper reaper_;

    ConnectionRepository connRepo_;
    EventHandlerRepository handlerRepo_;
};

#endif
