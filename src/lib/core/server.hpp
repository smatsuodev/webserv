#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "event/event_handler.hpp"
#include "transport/listener.hpp"
#include "server_state.hpp"

class Server {
public:
    explicit Server(const Endpoint &endpoint);
    ~Server();

    void start();

private:
    // NOTE: Server の作成と同時に初期化されるがよいか?
    Listener listener_;
    ServerState state_;

    void onHandlerError(const Context &ctx, error::AppError err);
    void onErrorEvent(const Event &event);
    static void executeActions(ActionContext &actionCtx, std::vector<IAction *> actions);
};

#endif
