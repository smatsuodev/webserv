#include "read_request_handler.hpp"
#include "write_response_handler.hpp"
#include "core/action.hpp"
#include "http/response/response.hpp"
#include "utils/logger.hpp"
#include "utils/io/reader.hpp"
#include "utils/types/try.hpp"

ReadRequestHandler::ReadRequestHandler(bufio::Reader &reader) : reqReader_(reader) {}

IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler");

    const RequestReader::ReadRequestResult result = reqReader_.readRequest();
    const http::Request req = TRY(result);

    LOG_DEBUGF("HTTP request parsed");

    // 今は同じ body をそのまま返す
    http::Headers headers;
    headers.insert(std::make_pair("Content-Length", utils::toString(req.getBody().size())));
    headers.insert(std::make_pair("Content-Type", "text/plain"));
    const http::Response res(http::kStatusOk, "HTTP/1.1", headers, req.getBody());

    std::vector<IAction *> actions;
    actions.push_back(new RegisterEventHandlerAction(ctx.getConnection().unwrap(), new WriteResponseHandler(res)));
    /**
     * クライアントはこれ以上イベントを起こさない可能性がある
     * 続きの handler が実行されるように、擬似的にイベントを通知する
     */
    actions.push_back(new TriggerPseudoEventAction(ctx.getEvent()));

    return Ok(actions);
}
