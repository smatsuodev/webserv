#include "action.hpp"
#include "http/handler/router.hpp"
#include "utils/logger.hpp"

void AddConnectionAction::execute(ActionContext &ctx) {
    ctx.getState().getConnectionRepository().set(conn_->getFd(), conn_);
}

void RemoveConnectionAction::execute(ActionContext &ctx) {
    ctx.getState().getConnectionRepository().remove(conn_.getFd());
}

void RegisterEventHandlerAction::execute(ActionContext &ctx) {
    ctx.getState().getEventHandlerRepository().set(conn_.getFd(), type_, handler_);
}

void UnregisterEventHandlerAction::execute(ActionContext &ctx) {
    ctx.getState().getEventHandlerRepository().remove(conn_.getFd(), type_);
}

void RegisterEventAction::execute(ActionContext &ctx) {
    ctx.getState().getEventNotifier().registerEvent(event_);
}

void UnregisterEventAction::execute(ActionContext &ctx) {
    ctx.getState().getEventNotifier().unregisterEvent(event_);
}
