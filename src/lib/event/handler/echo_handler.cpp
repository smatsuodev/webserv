#include "echo_handler.hpp"

#include "core/action.hpp"
#include "transport/connection.hpp"
#include "utils/logger.hpp"
#include "utils/io/reader.hpp"
#include "utils/types/try.hpp"
#include <sys/socket.h>

EchoHandler::InvokeResult EchoHandler::invoke(const Context &ctx) {
    LOG_DEBUGF("start EchoHandler::invoke");

    if (ctx.getConnection().isNone()) {
        LOG_INFO("no connection");
        return Err(error::kValidationInvalidArgs);
    }
    Connection *conn = ctx.getConnection().unwrap();

    const bufio::Reader::ReadAllResult result = conn->getReader().readAll();
    const std::string content = TRY(result);

    // TODO: send がブロックした場合のエラーハンドリング
    if (send(conn->getFd(), content.c_str(), content.size(), 0) == -1) {
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
