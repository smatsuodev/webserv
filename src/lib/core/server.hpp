#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "event/event_handler.hpp"
#include "transport/listener.hpp"
#include "server_state.hpp"
#include "virtual_server.hpp"
#include "config/config.hpp"
#include <set>

class Server {
public:
    explicit Server(const config::Config &config);
    ~Server();

    void start();

private:
    // VirtualServer は http::Router を持っていて、コピー不可なのでポインタで持つ
    typedef std::vector<VirtualServer *> VirtualServerList;

    static const int REQUEST_TIMEOUT_SECONDS = 5;
    static const int CGI_TIMEOUT_SECONDS = 5;

    config::Config config_;
    VirtualServerList virtualServers_;

    ServerState state_;
    // Listener がコピー不可なのでポインタで持つ
    std::vector<Listener *> listeners_;
    std::set<int> listenerFds_;

    void onHandlerError(const Context &ctx, error::AppError err);
    void onErrorEvent(const Event &event);
    static void executeActions(ActionContext &actionCtx, std::vector<IAction *> actions);
    void invokeHandlers(const Context &ctx);
    void invokeSingleHandler(const Context &ctx, const Ref<IEventHandler> &handler, bool shouldCallHandler);
    void removeTimeoutHandlers();
    void removeTimeoutRequestHandlers();
    void removeTimeoutCgiProcesses();
};

#endif
