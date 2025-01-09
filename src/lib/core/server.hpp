#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "event/event_handler.hpp"
#include "transport/listener.hpp"
#include "server_state.hpp"
#include "virtual_server.hpp"
#include "config/config.hpp"

class Server {
public:
    explicit Server(const config::Config &config);
    ~Server();

    void start();

private:
    // VirtualServer は http::Router を持っていて、コピー不可なのでポインタで持つ
    typedef std::vector<VirtualServer *> VirtualServerList;

    config::Config config_;
    VirtualServerList virtualServers_;

    ServerState state_;
    // Listener がコピー不可なのでポインタで持つ
    std::vector<Listener *> listeners_;
    std::set<int> listenerFds_;

    void onHandlerError(const Context &ctx, error::AppError err);
    void onErrorEvent(const Event &event);
    static void executeActions(ActionContext &actionCtx, std::vector<IAction *> actions);
};

#endif
