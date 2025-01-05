#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "event/event_handler.hpp"
#include "transport/listener.hpp"
#include "server_state.hpp"
#include "config/config.hpp"
#include "http/handler/router.hpp"

class Server {
public:
    explicit Server(const config::Config &config);
    ~Server();

    void start();

private:
    config::Config config_;
    config::Resolver resolver_;

    ServerState state_;
    // Listener がコピー不可なのでポインタで持つ
    std::vector<Listener *> listeners_;
    std::set<int> listenerFds_;

    void setupListeners();
    void onHandlerError(const Context &ctx, error::AppError err);
    void onErrorEvent(const Event &event);
    static void executeActions(ActionContext &actionCtx, std::vector<IAction *> actions);
    // router は各 virtual server 固有
    static http::Router createRouter(const config::ServerContext &serverConfig);
};

#endif
