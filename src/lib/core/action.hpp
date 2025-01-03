#ifndef SRC_LIB_CORE_ACTION_HPP
#define SRC_LIB_CORE_ACTION_HPP

#include "event/event_handler.hpp"
#include "http/handler/router.hpp"
#include "http/request/request.hpp"
#include "server_state.hpp"

// NOTE: router が必要なのは ServeHttpAction のみ。設計的に微妙?
class ActionContext {
public:
    ActionContext(ServerState &state, http::Router &router);

    ServerState &getState() const;
    http::Router &getRouter() const;

private:
    ServerState &state_;
    http::Router &router_;
};

class AddConnectionAction : public IAction {
public:
    explicit AddConnectionAction(Connection *conn);
    void execute(ActionContext &ctx);

private:
    Connection *conn_;
};

class RemoveConnectionAction : public IAction {
public:
    explicit RemoveConnectionAction(Connection &conn);
    void execute(ActionContext &ctx);

private:
    Connection &conn_;
    bool executed_;
};

class RegisterEventHandlerAction : public IAction {
public:
    RegisterEventHandlerAction(Connection &conn, IEventHandler *handler);
    void execute(ActionContext &ctx);

private:
    Connection &conn_;
    IEventHandler *handler_;
    bool executed_;
};

class UnregisterEventHandlerAction : public IAction {
public:
    explicit UnregisterEventHandlerAction(Connection &conn);
    void execute(ActionContext &ctx);

private:
    Connection &conn_;
    bool executed_;
};

class RegisterEventAction : public IAction {
public:
    explicit RegisterEventAction(const Event &event);
    void execute(ActionContext &ctx);

private:
    Event event_;
    bool executed_;
};

class UnregisterEventAction : public IAction {
public:
    explicit UnregisterEventAction(const Event &event);
    void execute(ActionContext &ctx);

private:
    Event event_;
    bool executed_;
};

class ServeHttpAction : public IAction {
public:
    explicit ServeHttpAction(const Context &eventCtx, const http::Request &req);
    void execute(ActionContext &actionCtx);

private:
    Context eventCtx_;
    http::Request req_;
    bool executed_;
};

#endif
