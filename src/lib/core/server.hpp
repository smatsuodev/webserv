#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "event/handler/event_handler.hpp"
#include "transport/connection.hpp"
#include <map>

class Server {
public:
    Server();
    ~Server();

    void start(unsigned short port);
    void addConnection(Connection *conn);
    void registerEventHandler(int targetFd, IEventHandler *handler);

private:
    std::map<int, Connection *> connections_;
    std::map<int, IEventHandler *> eventHandlers_;

    enum HandleConnectionState {
        kSuspend,
        kComplete,
    };
    static Result<HandleConnectionState, error::AppError> handleConnection(const Connection &conn);
};

#endif
