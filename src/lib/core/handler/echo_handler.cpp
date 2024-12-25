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

    std::string content;
    Option<error::AppError> err = None;
    while (true) {
        char buffer[1024];
        const io::IReader::ReadResult result = conn->getReader().read(buffer, sizeof(buffer));
        if (result.isErr()) {
            err = Some(result.unwrapErr());
            break;
        }
        const size_t bytesRead = result.unwrap();
        if (bytesRead == 0) {
            break;
        }
        content.append(buffer, bytesRead);
    }

    // TODO: send がブロックした場合のエラーハンドリング
    // err があっても、途中まで読み込んだものは書き込む
    if (!content.empty() && send(conn->getFd(), content.c_str(), content.size(), 0) == -1) {
        LOG_WARN("failed to send response");
        return Err(error::kUnknown);
    }

    if (err.isSome()) {
        if (err.unwrap() == error::kIOWouldBlock) {
            LOG_DEBUGF("read would block");
        }
        return Err(err.unwrap());
    }

    LOG_DEBUG("response sent");

    std::vector<IAction *> actions;
    actions.push_back(new UnregisterEventAction(ctx.getEvent()));
    actions.push_back(new UnregisterEventHandlerAction(conn, this));
    actions.push_back(new RemoveConnectionAction(ctx.getConnection().unwrap()));

    return Ok(actions);
}
