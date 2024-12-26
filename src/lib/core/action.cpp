#include "action.hpp"

#include "server.hpp"

IAction::~IAction() {}

AddConnectionAction::AddConnectionAction(Connection *conn) : conn_(conn) {}

void AddConnectionAction::execute(ServerState &state) {
    if (conn_) {
        state.getConnectionRepository().set(conn_->getFd(), conn_);
        // 2回以上実行できないようにする
        conn_ = NULL;
    }
}

RemoveConnectionAction::RemoveConnectionAction(Connection *conn) : conn_(conn) {}

void RemoveConnectionAction::execute(ServerState &state) {
    if (conn_) {
        state.getConnectionRepository().remove(conn_->getFd());
        conn_ = NULL;
    }
}

RegisterEventHandlerAction::RegisterEventHandlerAction(Connection *conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void RegisterEventHandlerAction::execute(ServerState &state) {
    if (!executed_) {
        state.getEventHandlerRepository().set(conn_->getFd(), handler_);
        executed_ = true;
    }
}

UnregisterEventHandlerAction::UnregisterEventHandlerAction(Connection *conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void UnregisterEventHandlerAction::execute(ServerState &state) {
    if (!executed_) {
        state.getEventHandlerRepository().remove(conn_->getFd());
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
