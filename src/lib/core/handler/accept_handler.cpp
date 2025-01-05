#include "accept_handler.hpp"
#include "read_request_handler.hpp"
#include "core/action.hpp"
#include "core/server.hpp"
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

    // caller が処理すべき action を返す
    std::vector<IAction *> actions;
    actions.push_back(new RegisterEventAction(eventToRegister));
    actions.push_back(new AddConnectionAction(newConnection));
    actions.push_back(new RegisterEventHandlerAction(*newConnection, new ReadRequestHandler(ctx)));

    return Ok(actions);
}
