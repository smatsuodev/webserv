#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "event/event_notifier.hpp"
#include "event/handler/event_handler.hpp"
#include "transport/connection.hpp"
#include <map>

class Server {
public:
    Server();
    ~Server();

    void start(unsigned short port);
    void addConnection(Connection *conn);
    void removeConnection(const Connection *conn);
    void registerEventHandler(int targetFd, IEventHandler *handler);
    EventNotifier &getEventNotifier();

private:
    // NOTE: Server の作成と一緒に EventNotifier が初期化される。それでよいのか?
    EventNotifier notifier_;
    std::map<int, Connection *> connections_;
    std::map<int, IEventHandler *> eventHandlers_;

    Option<Connection *> findConnection(int fd) const;
    Option<IEventHandler *> findEventHandler(int fd);
};

#endif
