#include "read_request_handler.hpp"

#include "echo_handler.hpp"
#include "core/action.hpp"
#include "http/request_parser.hpp"
#include "transport/connection.hpp"
#include "utils/logger.hpp"
#include "utils/string.hpp"
#include "utils/io/reader.hpp"
#include "utils/types/try.hpp"

// TODO: 途中でブロックしてもいいように、データを保持する
IEventHandler::InvokeResult ReadRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadRequestHandler::invoke");

    Connection *conn = ctx.getConnection().unwrap();
    bufio::Reader &reader = conn->getReader();

    // request-line CRLF
    const bufio::Reader::ReadUntilResult readRequestLineResult = reader.readUntil("\r\n");
    std::string requestLine = TRY(readRequestLineResult);
    if (!utils::endsWith(requestLine, "\r\n")) {
        LOG_DEBUG("start-line does not end with CRLF");
        return Err(error::kParseUnknown);
    }

    requestLine.erase(requestLine.size() - 2); // remove CRLF

    // *( field-line CRLF ) CRLF
    std::vector<std::string> headers;
    Option<size_t> contentLength = None;
    while (true) {
        const bufio::Reader::ReadUntilResult readHeaderResult = reader.readUntil("\r\n");
        std::string header = TRY(readHeaderResult);
        if (!utils::endsWith(header, "\r\n")) {
            LOG_DEBUG("field-line does not end with CRLF");
            return Err(error::kParseUnknown);
        }

        if (header == "\r\n") {
            break;
        }

        header.erase(header.size() - 2); // remove CRLF
        headers.push_back(header);

        // Content-Length ヘッダーの値を取得
        const http::RequestParser::ParseHeaderFieldLineResult parseHeaderResult =
            http::RequestParser::parseHeaderFieldLine(header);
        const http::RequestParser::HeaderField &field = TRY(parseHeaderResult);
        if (field.first == "Content-Length") {
            // TODO: client_max_body_size より大きい値の場合はエラー
            const utils::StoulResult result = utils::stoul(field.second);
            contentLength = Some(TRY(result));
        }
    }

    Option<std::string> maybeBody = None;
    if (contentLength.isSome()) {
        // message-body
        const size_t &bodySize = contentLength.unwrap();
        char *buf = new char[bodySize];
        const bufio::Reader::ReadResult readResult = reader.read(buf, bodySize);
        const size_t bytesRead = TRY(readResult);
        maybeBody = Some(std::string(buf, bytesRead));
        delete[] buf;
    }

    const http::RequestParser::ParseResult parseResult =
        http::RequestParser::parseRequest(requestLine, headers, maybeBody.unwrapOr(""));
    http::Request request = TRY(parseResult);

    LOG_DEBUGF("HTTP request parsed");

    // event handler はイベントが発生するまで実行されないので、ブロックしない限り実行を続ける
    // 現状は同じ body を返す
    std::auto_ptr<IEventHandler> nextHandler(new EchoHandler(request.getBody()));
    InvokeResult nextHandlerResult = nextHandler->invoke(ctx);
    if (nextHandlerResult.isErr()) {
        const error::AppError err = nextHandlerResult.unwrapErr();
        if (err == error::kIOWouldBlock) {
            // 再試行するために登録
            std::vector<IAction *> actions;
            actions.push_back(new RegisterEventHandlerAction(conn, nextHandler.release()));
            return Ok(actions);
        }
        return Err(err);
    }

    // 成功したら EchoHandler が UnregisterEventHandlerAction を発行するので、所有権を破棄する
    nextHandler.release();
    return Ok(nextHandlerResult.unwrap());
}
