#include "action.hpp"

IAction::~IAction() {}

AddConnectionAction::AddConnectionAction(Connection *conn) : conn_(conn) {}

void AddConnectionAction::execute(Server &server) {
    if (conn_) {
        server.connRepo_.set(conn_->getFd(), conn_);
        // 2回以上実行できないようにする
        conn_ = NULL;
    }
}

RemoveConnectionAction::RemoveConnectionAction(Connection *conn) : conn_(conn) {}

void RemoveConnectionAction::execute(Server &server) {
    if (conn_) {
        server.connRepo_.remove(conn_->getFd());
        conn_ = NULL;
    }
}

UnregisterEventHandlerAction::UnregisterEventHandlerAction(Connection *conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void UnregisterEventHandlerAction::execute(Server &server) {
    if (!executed_) {
        server.handlerRepo_.remove(conn_->getFd());
        executed_ = true;
    }
}

RegisterEventAction::RegisterEventAction(const Event &event) : event_(event), executed_(false) {}

void RegisterEventAction::execute(Server &server) {
    if (!executed_) {
        server.notifier_.registerEvent(event_);
        executed_ = true;
    }
}

UnregisterEventAction::UnregisterEventAction(const Event &event) : event_(event), executed_(false) {}

void UnregisterEventAction::execute(Server &server) {
    if (!executed_) {
        server.notifier_.unregisterEvent(event_);
        executed_ = true;
    }
}

RegisterEventHandlerAction::RegisterEventHandlerAction(Connection *conn, IEventHandler *handler)
    : conn_(conn), handler_(handler), executed_(false) {}

void RegisterEventHandlerAction::execute(Server &server) {
    if (!executed_) {
        server.handlerRepo_.set(conn_->getFd(), handler_);
        executed_ = true;
    }
}
