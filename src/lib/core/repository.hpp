#ifndef SRC_LIB_CORE_REPOSITORY_HPP
#define SRC_LIB_CORE_REPOSITORY_HPP

#include "event/event_handler.hpp"
#include "transport/connection.hpp"
#include "utils/non_copyable.hpp"
#include <map>

// TODO: 共通化するべき?

class ConnectionRepository : public NonCopyable {
public:
    ConnectionRepository();
    ~ConnectionRepository();

    friend class AddConnectionAction;
    friend class RemoveConnectionAction;

    // NOTE: 参照を返すべきか?
    Option<Connection *> get(int fd);
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

    friend class RegisterEventHandlerAction;
    friend class UnRegisterEventHandlerAction;

    Option<IEventHandler *> get(int fd);
    void set(int fd, IEventHandler *handler);
    void remove(int fd);

private:
    std::map<int, IEventHandler *> handlers_;
};

#endif
