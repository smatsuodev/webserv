#include "read_request_handler.hpp"
#include "core/action.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

ReadRequestHandler::ReadRequestHandler(const Context &ctx)
    : resolver_(
          new RequestReader::ConfigResolver(ctx.getResolver(), ctx.getConnection().unwrap().get().getLocalAddress())
      ),
      reqReader_(*resolver_) {}

IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler");

    ReadBuffer &readBuf = ctx.getConnection().unwrap().get().getReadBuffer();
    const Option<http::Request> req = TRY(reqReader_.readRequest(readBuf));
    if (req.isNone()) {
        // read は高々 1 回なので、パースまで完了しなかった
        LOG_DEBUG("request is not fully read");
        return Err(error::kRecoverable);
    }

    LOG_DEBUGF("HTTP request parsed");

    std::vector<IAction *> actions;
    actions.push_back(new RegisterEventAction(Event(ctx.getEvent().getFd(), Event::kWrite)));
    actions.push_back(new ServeHttpAction(ctx, req.unwrap()));

    return Ok(actions);
}
