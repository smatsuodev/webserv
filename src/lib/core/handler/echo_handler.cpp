#include "echo_handler.hpp"
#include "core/action.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"
#include <sys/socket.h>

EchoHandler::EchoHandler(const std::string &content) : content_(content) {}

EchoHandler::InvokeResult EchoHandler::invoke(const Context &ctx) {
    LOG_DEBUGF("start EchoHandler");

    if (ctx.getConnection().isNone()) {
        LOG_INFO("no connection");
        return Err(error::kValidationInvalidArgs);
    }
    Connection *conn = ctx.getConnection().unwrap();

    const std::string response =
        utils::format("HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %zu\r\n\r\n%s", content_.size(),
                      content_.c_str());

    // TODO: send がブロックした場合のエラーハンドリング
    if (send(conn->getFd(), response.c_str(), response.size(), 0) == -1) {
        LOG_WARN("failed to send response");
        return Err(error::kUnknown);
    }

    LOG_DEBUG("response sent");

    std::vector<IAction *> actions;
    actions.push_back(new UnregisterEventAction(ctx.getEvent()));
    actions.push_back(new UnregisterEventHandlerAction(conn, this));
    actions.push_back(new RemoveConnectionAction(ctx.getConnection().unwrap()));

    return Ok(actions);
}
