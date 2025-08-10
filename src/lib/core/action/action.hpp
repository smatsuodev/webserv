#ifndef SRC_LIB_CORE_ACTION_ACTION_HPP
#define SRC_LIB_CORE_ACTION_ACTION_HPP

#include "event/event_handler.hpp"
#include "http/request/request.hpp"
#include "../server_state.hpp"
#include "../virtual_server_resolver.hpp"
#include "cgi/request.hpp"

class ActionContext {
public:
    explicit ActionContext(ServerState &state) : state_(state) {}
    ServerState &getState() const {
        return state_;
    }

private:
    ServerState &state_;
};

class AddConnectionAction : public IAction {
public:
    explicit AddConnectionAction(Connection *conn) : conn_(conn) {}
    void execute(ActionContext &ctx);

private:
    Connection *conn_;
};

class RemoveConnectionAction : public IAction {
public:
    explicit RemoveConnectionAction(Connection &conn) : conn_(conn) {}
    void execute(ActionContext &ctx);

private:
    Connection &conn_;
};

class RegisterEventHandlerAction : public IAction {
public:
    RegisterEventHandlerAction(Connection &conn, Event::EventType type, IEventHandler *handler)
        : conn_(conn), type_(type), handler_(handler) {}
    void execute(ActionContext &ctx);

private:
    Connection &conn_;
    Event::EventType type_;
    IEventHandler *handler_;
};

class UnregisterEventHandlerAction : public IAction {
public:
    explicit UnregisterEventHandlerAction(Connection &conn, Event::EventType type) : conn_(conn), type_(type) {}
    void execute(ActionContext &ctx);

private:
    Connection &conn_;
    Event::EventType type_;
};

class RegisterEventAction : public IAction {
public:
    explicit RegisterEventAction(const Event &event) : event_(event) {}
    void execute(ActionContext &ctx);

private:
    Event event_;
};

class UnregisterEventAction : public IAction {
public:
    explicit UnregisterEventAction(const Event &event) : event_(event) {}
    void execute(ActionContext &ctx);

private:
    Event event_;
};

class ServeHttpAction : public IAction {
public:
    typedef IEventHandler *(EventHandlerFactory)(const http::Response &);

    ServeHttpAction(const Context &eventCtx, const http::Request &req, EventHandlerFactory *factory)
        : eventCtx_(eventCtx), req_(req), factory_(factory) {}
    void execute(ActionContext &actionCtx);

private:
    Context eventCtx_;
    http::Request req_;
    EventHandlerFactory *factory_;
};

class RunCgiAction : public IAction {
public:
    explicit RunCgiAction(const cgi::Request &cgiRequest, int clientFd)
        : cgiRequest_(cgiRequest), clientFd_(clientFd) {}
    void execute(ActionContext &ctx);

private:
    cgi::Request cgiRequest_;
    int clientFd_;
};

#endif
