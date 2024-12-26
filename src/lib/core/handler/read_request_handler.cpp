#include "read_request_handler.hpp"
#include "echo_handler.hpp"
#include "core/action.hpp"
#include "utils/logger.hpp"
#include "utils/io/reader.hpp"
#include "utils/types/try.hpp"

ReadRequestHandler::ReadRequestHandler(bufio::Reader &reader) : reqReader_(reader) {}

IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler");

    const RequestReader::ReadRequestResult result = reqReader_.readRequest();
    const http::Request req = TRY(result);

    LOG_DEBUGF("HTTP request parsed");

    // event handler はイベントが発生するまで実行されないので、ブロックしない限り実行を続ける
    // TODO: 擬似的にイベントを起こせば不要になる?
    // 現状は同じ body を返す
    std::auto_ptr<IEventHandler> nextHandler(new EchoHandler(req.getBody()));
    const InvokeResult nextHandlerResult = nextHandler->invoke(ctx);
    if (nextHandlerResult.isErr()) {
        const error::AppError err = nextHandlerResult.unwrapErr();
        if (err == error::kIOWouldBlock) {
            // 再試行するために登録
            std::vector<IAction *> actions;
            actions.push_back(new RegisterEventHandlerAction(ctx.getConnection().unwrap(), nextHandler.release()));
            return Ok(actions);
        }
        return Err(err);
    }

    // 成功したら EchoHandler が UnregisterEventHandlerAction を発行するので、所有権を破棄する
    nextHandler.release();
    return Ok(nextHandlerResult.unwrap());
}
