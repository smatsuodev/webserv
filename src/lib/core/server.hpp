#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "repository.hpp"
#include "event/event_notifier.hpp"
#include "event/event_handler.hpp"
#include "transport/listener.hpp"

class ServerState {
public:
    EventNotifier &getEventNotifier();
    ConnectionRepository &getConnectionRepository();
    EventHandlerRepository &getEventHandlerRepository();

private:
    // EventNotifier はあんまり state っぽくない
    EventNotifier notifier_;
    ConnectionRepository connRepo_;
    EventHandlerRepository handlerRepo_;
};

class Server {
public:
    Server(const std::string &ip, unsigned short port);
    ~Server();

    void start();

private:
    std::string ip_;
    unsigned short port_;

    // NOTE: Server の作成と同時に初期化されるがよいか?
    Listener listener_;
    ServerState state_;

    void onHandlerError(const Context &ctx, error::AppError err);
    void executeActions(std::vector<IAction *> actions);
};

#endif
