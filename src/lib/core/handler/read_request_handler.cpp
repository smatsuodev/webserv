#include "read_request_handler.hpp"
#include "write_response_handler.hpp"
#include "core/action.hpp"
#include "http/handler/handler.hpp"
#include "http/response/response.hpp"
#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

ReadRequestHandler::ReadRequestHandler(ReadBuffer &readBuf) : reqReader_(readBuf) {}

IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler");

    const Option<http::Request> req = TRY(reqReader_.readRequest());
    if (req.isNone()) {
        // read は高々 1 回なので、パースまで完了しなかった
        LOG_DEBUG("request is not fully read");
        return Err(error::kRecoverable);
    }

    LOG_DEBUGF("HTTP request parsed");

    // TODO: すべてのエラーが 404 とは限らない. 適切なレスポンスを返すようにする
    // TODO: readFile が kWouldBlock を返すと、読み取った req が失われる
    const http::Response res = http::Handler()
                                   .serve(req.unwrap())
                                   .unwrapOr(http::ResponseBuilder().text("not found", http::kStatusNotFound).build());

    std::vector<IAction *> actions;
    actions.push_back(new RegisterEventAction(Event(ctx.getEvent().getFd(), Event::kWrite)));
    actions.push_back(new RegisterEventHandlerAction(ctx.getConnection().unwrap(), new WriteResponseHandler(res)));

    return Ok(actions);
}
