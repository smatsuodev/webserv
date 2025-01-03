#include "action.hpp"
#include "server.hpp"
#include "handler/write_response_handler.hpp"
#include "http/handler/router.hpp"

IAction::~IAction() {}

AddConnectionAction::AddConnectionAction(Connection *conn) : conn_(conn) {}

void AddConnectionAction::execute(ServerState &state) {
    if (conn_) {
        state.getConnectionRepository().set(conn_->getFd(), conn_);
        // 2回以上実行できないようにする
        conn_ = NULL;
    }
}

RemoveConnectionAction::RemoveConnectionAction(Connection &conn) : conn_(conn), executed_(false) {}

void RemoveConnectionAction::execute(ServerState &state) {
    if (!executed_) {
        state.getConnectionRepository().remove(conn_.getFd());
        executed_ = true;
    }
}

RegisterEventHandlerAction::RegisterEventHandlerAction(Connection &conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void RegisterEventHandlerAction::execute(ServerState &state) {
    if (!executed_) {
        state.getEventHandlerRepository().set(conn_.getFd(), handler_);
        executed_ = true;
    }
}

UnregisterEventHandlerAction::UnregisterEventHandlerAction(Connection &conn) : conn_(conn), executed_(false) {}

void UnregisterEventHandlerAction::execute(ServerState &state) {
    if (!executed_) {
        state.getEventHandlerRepository().remove(conn_.getFd());
        executed_ = true;
    }
}

RegisterEventAction::RegisterEventAction(const Event &event) : event_(event), executed_(false) {}

void RegisterEventAction::execute(ServerState &state) {
    if (!executed_) {
        state.getEventNotifier().registerEvent(event_);
        executed_ = true;
    }
}

UnregisterEventAction::UnregisterEventAction(const Event &event) : event_(event), executed_(false) {}

void UnregisterEventAction::execute(ServerState &state) {
    if (!executed_) {
        state.getEventNotifier().unregisterEvent(event_);
        executed_ = true;
    }
}

ServeHttpAction::ServeHttpAction(const Context &ctx, const http::Request &req)
    : ctx_(ctx), req_(req), executed_(false) {}

void ServeHttpAction::execute(ServerState &state) {
    if (executed_) {
        return;
    }
    executed_ = true;

    // TODO: 構築済みの router を使い回すようにする
    http::Router router;
    router.onGet("/", new http::Handler());
    router.onDelete("/", new http::Handler());

    const Connection &conn = ctx_.getConnection().unwrap();
    const http::Response res = router.serve(req_);
    state.getEventHandlerRepository().set(conn.getFd(), new WriteResponseHandler(res));
}
