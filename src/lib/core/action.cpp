#include "action.hpp"

IAction::~IAction() {}

AddConnectionAction::AddConnectionAction(Connection *conn) : conn_(conn) {}

void AddConnectionAction::execute(Server &server) {
    if (conn_) {
        server.addConnection(conn_);
        // 2回以上実行できないようにする
        conn_ = NULL;
    }
}

RemoveConnectionAction::RemoveConnectionAction(Connection *conn) : conn_(conn) {}

void RemoveConnectionAction::execute(Server &server) {
    if (conn_) {
        server.removeConnection(conn_);
        conn_ = NULL;
    }
}

RegisterEventAction::RegisterEventAction(const Event &event) : event_(event), executed_(false) {}

void RegisterEventAction::execute(Server &server) {
    if (!executed_) {
        server.getEventNotifier().registerEvent(event_);
        executed_ = true;
    }
}

UnregisterEventAction::UnregisterEventAction(const Event &event) : event_(event), executed_(false) {}

void UnregisterEventAction::execute(Server &server) {
    if (!executed_) {
        server.getEventNotifier().unregisterEvent(event_);
        executed_ = true;
    }
}

RegisterEventHandlerAction::RegisterEventHandlerAction(Connection *conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void RegisterEventHandlerAction::execute(Server &server) {
    if (!executed_) {
        server.registerEventHandler(conn_->getFd(), handler_);
        executed_ = true;
    }
}
