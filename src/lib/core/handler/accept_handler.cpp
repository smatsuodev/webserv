#include "accept_handler.hpp"
#include "read_request_handler.hpp"
#include "core/action/action.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

AcceptHandler::AcceptHandler(Listener &listener) : listener_(listener) {}

IEventHandler::InvokeResult AcceptHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start AcceptHandler");

    // TODO: accept がブロックしたら?
    const Listener::AcceptConnectionResult result = listener_.acceptConnection();
    if (result.isErr()) {
        LOG_WARNF("failed to accept connection: %s", result.unwrapErr().c_str());
        return Err(error::kUnknown);
    }

    Connection *newConnection = result.unwrap();
    const Event eventToRegister = Event(newConnection->getFd(), Event::kRead);
    /**
     * actions を Server が実行して初めて Context に Connection が含まれるようになる
     * Connection を含む Context が先にほしいので、一時的に作る
     */
    const Context newCtx = ctx.withNewValues(eventToRegister, Some(utils::ref(*newConnection)));

    // caller が処理すべき action を返す
    std::vector<IAction *> actions;
    actions.push_back(new RegisterEventAction(eventToRegister));
    actions.push_back(new AddConnectionAction(newConnection));
    actions.push_back(new RegisterEventHandlerAction(
        *newConnection, Event::kRead, new ReadRequestHandler(newCtx.getResolver().unwrap())
    ));

    return Ok(actions);
}
