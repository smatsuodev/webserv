#include "./action.hpp"
#include "utils/logger.hpp"

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
