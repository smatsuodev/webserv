#include "write_response_handler.hpp"
#include "core/action.hpp"
#include "utils/logger.hpp"
#include <unistd.h>

WriteResponseHandler::WriteResponseHandler(const http::Response &response) : response_(response) {}

IEventHandler::InvokeResult WriteResponseHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start WriteResponseHandler::invoke");

    Connection *conn = ctx.getConnection().unwrap();
    const std::string &message = response_.toString();

    // TODO: ブロックした場合の処理
    if (write(conn->getFd(), message.c_str(), message.size()) == -1) {
        LOG_WARN("failed to write response");
        return Err(error::kIOUnknown);
    }

    LOG_DEBUG("response written");

    std::vector<IAction *> actions;
    actions.push_back(new UnregisterEventAction(ctx.getEvent()));
    actions.push_back(new UnregisterEventHandlerAction(conn, this));
    actions.push_back(new RemoveConnectionAction(ctx.getConnection().unwrap()));

    return Ok(actions);
}
