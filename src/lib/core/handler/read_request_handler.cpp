#include "read_request_handler.hpp"
#include "write_response_body_handler.hpp"
#include "core/action/action.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

// http::Response は ServeHttpAction で生成されるので、event handler を生成する関数を渡す
IEventHandler *writeResponseHandlerFactory(const http::Response &res) {
    return new WriteResponseHandler(res);
}

ReadRequestHandler::ReadRequestHandler(const VirtualServerResolver &vsResolver)
    : resolver_(new ConfigResolver(vsResolver)), reqReader_(*resolver_) {}

IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler");

    // アクティビティを更新
    ctx.getConnection().unwrap().get().updateActivity();
    
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
    actions.push_back(new UnregisterEventHandlerAction(ctx.getConnection().unwrap(), Event::kRead));
    actions.push_back(new ServeHttpAction(ctx, req.unwrap(), writeResponseHandlerFactory));

    return Ok(actions);
}

ConfigResolver::ConfigResolver(const VirtualServerResolver &vsResolver) : resolver_(vsResolver) {}

Option<config::ServerContext> ConfigResolver::resolve(const std::string &host) const {
    const Option<Ref<VirtualServer> > vs = resolver_.resolve(host);
    if (vs.isNone()) {
        return None;
    }
    return Some(vs.unwrap().get().getServerConfig());
}
