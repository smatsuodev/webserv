#include "action.hpp"
#include "http/handler/router.hpp"
#include "utils/logger.hpp"

IAction::~IAction() {}

ActionContext::ActionContext(ServerState &state) : state_(state) {}

ServerState &ActionContext::getState() const {
    return state_;
}

AddConnectionAction::AddConnectionAction(Connection *conn) : conn_(conn) {}

void AddConnectionAction::execute(ActionContext &ctx) {
    ctx.getState().getConnectionRepository().set(conn_->getFd(), conn_);
}

RemoveConnectionAction::RemoveConnectionAction(Connection &conn) : conn_(conn) {}

void RemoveConnectionAction::execute(ActionContext &ctx) {
    ctx.getState().getConnectionRepository().remove(conn_.getFd());
}

RegisterEventHandlerAction::RegisterEventHandlerAction(Connection &conn, IEventHandler *handler)
    : conn_(conn), handler_(handler) {}

void RegisterEventHandlerAction::execute(ActionContext &ctx) {
    ctx.getState().getEventHandlerRepository().set(conn_.getFd(), handler_);
}

UnregisterEventHandlerAction::UnregisterEventHandlerAction(Connection &conn) : conn_(conn) {}

void UnregisterEventHandlerAction::execute(ActionContext &ctx) {
    ctx.getState().getEventHandlerRepository().remove(conn_.getFd());
}

RegisterEventAction::RegisterEventAction(const Event &event) : event_(event) {}

void RegisterEventAction::execute(ActionContext &ctx) {
    ctx.getState().getEventNotifier().registerEvent(event_);
}

UnregisterEventAction::UnregisterEventAction(const Event &event) : event_(event) {}

void UnregisterEventAction::execute(ActionContext &ctx) {
    ctx.getState().getEventNotifier().unregisterEvent(event_);
}

ServeHttpAction::ServeHttpAction(const Context &eventCtx, const http::Request &req, EventHandlerFactory *factory)
    : eventCtx_(eventCtx), req_(req), factory_(factory) {}

void ServeHttpAction::execute(ActionContext &actionCtx) {
    const Connection &conn = eventCtx_.getConnection().unwrap();
    // Host ヘッダーは必須なので存在するはず
    const std::string hostHeader = req_.getHeader("Host").unwrap();
    // accept 後は resolver は存在するはず
    const Option<Ref<VirtualServer> > vs = eventCtx_.getResolver().unwrap().resolve(hostHeader);

    // accept したので Connection, VirtualServer は存在するはず
    const http::RequestContext reqContext(req_, eventCtx_.getConnection().unwrap());
    const Either<IAction *, http::Response> resOrAction = vs.unwrap().get().getRouter().serve(reqContext);
    if (resOrAction.isLeft()) {
        LOG_DEBUG("ServeHttpAction: action is returned");
        IAction *action = resOrAction.unwrapLeft();
        action->execute(actionCtx);
        delete action;
        return;
    }

    IEventHandler *nextHandler = factory_(resOrAction.unwrapRight());
    actionCtx.getState().getEventHandlerRepository().set(conn.getFd(), nextHandler);
}
