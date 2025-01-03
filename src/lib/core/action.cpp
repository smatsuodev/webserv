#include "action.hpp"
#include "server.hpp"
#include "handler/write_response_handler.hpp"
#include "http/handler/router.hpp"

IAction::~IAction() {}

ActionContext::ActionContext(ServerState &state, http::Router &router) : state_(state), router_(router) {}

ServerState &ActionContext::getState() const {
    return state_;
}

http::Router &ActionContext::getRouter() const {
    return router_;
}

AddConnectionAction::AddConnectionAction(Connection *conn) : conn_(conn) {}

void AddConnectionAction::execute(ActionContext &ctx) {
    if (conn_) {
        ctx.getState().getConnectionRepository().set(conn_->getFd(), conn_);
        // 2回以上実行できないようにする
        conn_ = NULL;
    }
}

RemoveConnectionAction::RemoveConnectionAction(Connection &conn) : conn_(conn), executed_(false) {}

void RemoveConnectionAction::execute(ActionContext &ctx) {
    if (!executed_) {
        ctx.getState().getConnectionRepository().remove(conn_.getFd());
        executed_ = true;
    }
}

RegisterEventHandlerAction::RegisterEventHandlerAction(Connection &conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void RegisterEventHandlerAction::execute(ActionContext &ctx) {
    if (!executed_) {
        ctx.getState().getEventHandlerRepository().set(conn_.getFd(), handler_);
        executed_ = true;
    }
}

UnregisterEventHandlerAction::UnregisterEventHandlerAction(Connection &conn) : conn_(conn), executed_(false) {}

void UnregisterEventHandlerAction::execute(ActionContext &ctx) {
    if (!executed_) {
        ctx.getState().getEventHandlerRepository().remove(conn_.getFd());
        executed_ = true;
    }
}

RegisterEventAction::RegisterEventAction(const Event &event) : event_(event), executed_(false) {}

void RegisterEventAction::execute(ActionContext &ctx) {
    if (!executed_) {
        ctx.getState().getEventNotifier().registerEvent(event_);
        executed_ = true;
    }
}

UnregisterEventAction::UnregisterEventAction(const Event &event) : event_(event), executed_(false) {}

void UnregisterEventAction::execute(ActionContext &ctx) {
    if (!executed_) {
        ctx.getState().getEventNotifier().unregisterEvent(event_);
        executed_ = true;
    }
}

ServeHttpAction::ServeHttpAction(const Context &eventCtx, const http::Request &req)
    : eventCtx_(eventCtx), req_(req), executed_(false) {}

void ServeHttpAction::execute(ActionContext &actionCtx) {
    if (executed_) {
        return;
    }
    executed_ = true;

    const Connection &conn = eventCtx_.getConnection().unwrap();
    const http::Response res = actionCtx.getRouter().serve(req_);
    actionCtx.getState().getEventHandlerRepository().set(conn.getFd(), new WriteResponseHandler(res));
}
