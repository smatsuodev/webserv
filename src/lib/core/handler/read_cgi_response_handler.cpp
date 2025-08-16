#include "read_cgi_response_handler.hpp"

#include <unistd.h>
#include <sys/wait.h>

#include "../action/action.hpp"
#include "../../event/event.hpp"
#include "../../transport/connection.hpp"
#include "../../utils/logger.hpp"
#include "../../http/response/response_builder.hpp"
#include "write_response_body_handler.hpp"
#include "../../utils/types/option.hpp"
#include "../../http/request/request_parser.hpp"

#include <sys/poll.h>

ReadCgiResponseHandler::ReadCgiResponseHandler(const int clientFd, const pid_t childPid)
    : cgiResponse_(None), clientFd_(clientFd), childPid_(childPid) {}

ReadCgiResponseHandler::~ReadCgiResponseHandler() {}

IEventHandler::InvokeResult ReadCgiResponseHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadCgiResponseHandler::invoke");

    Connection &conn = ctx.getConnection().unwrap();

    char buffer[4096];
    const ssize_t bytesRead = read(conn.getFd(), buffer, sizeof(buffer));
    if (bytesRead == -1) {
        LOG_WARN("failed to read CGI response");
        return Err(error::AppError::kUnknown);
    }

    if (bytesRead != 0) {
        // データを蓄積
        responseBuffer_.append(buffer, bytesRead);
        LOG_DEBUGF("CGI response: %zd bytes read, total: %zu", bytesRead, responseBuffer_.size());

        // まだデータが来る可能性がある
        return Err(error::kRecoverable);
    }

    // NOTE: ほんまに?
    // EOF - CGIプロセスが終了
    LOG_DEBUG("CGI process finished");

    // CGIレスポンスをパース
    const Result<cgi::Response, error::AppError> result = createCgiResponseFromBuffer(responseBuffer_);
    if (result.isOk()) {
        cgiResponse_ = Some(result.unwrap());
    }

    // HTTPレスポンスを生成
    http::ResponseBuilder builder;

    // status, body を設定
    if (cgiResponse_.isSome()) {
        const cgi::Response &response = cgiResponse_.unwrap();
        http::HttpStatusCode statusCode = determineStatusCode(response);

        builder.status(statusCode);
        if (response.getBody().isSome()) {
            builder.body(response.getBody().unwrap(), statusCode);
        }
    }

    // ヘッダーを設定
    if (cgiResponse_.isSome()) {
        const cgi::Response &response = cgiResponse_.unwrap();
        for (std::map<std::string, std::string>::const_iterator it = response.getHeaders().begin();
             it != response.getHeaders().end();
             ++it) {
            // Statusヘッダーは既に処理済みなのでスキップ
            if (it->first == "Status") continue;
            builder.header(it->first, it->second);
        }
    }

    const http::Response httpResponse = builder.build();
    const std::string responseMessage = httpResponse.toString();

    // 子プロセスの回収
    int status;
    waitpid(childPid_, &status, WNOHANG);

    return Ok(makeNextActions(conn, httpResponse));
}

IEventHandler::ErrorHandleResult ReadCgiResponseHandler::onErrorEvent(const Context &, const Event &event) {
    LOG_WARNF("ReadCgiResponseHandler received an error event %d", event.getTypeFlags());

    if (event.getTypeFlags() & Event::kHangUp) {
        return ErrorHandleResult(false, std::vector<IAction *>());
    }

    return ErrorHandleResult();
}

std::vector<IAction *>
ReadCgiResponseHandler::makeNextActions(Connection &conn, const http::Response &httpResponse) const {
    // クライアントへレスポンスを送信するアクションを作成
    std::vector<IAction *> actions;

    // CGIソケットのクリーンアップ
    actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kRead)));
    actions.push_back(new UnregisterEventHandlerAction(conn, Event::kRead));
    actions.push_back(new RemoveConnectionAction(conn));

    // クライアントへのレスポンス送信
    // clientFd_に対してWriteイベントとハンドラを登録
    actions.push_back(new RegisterEventAction(Event(clientFd_, Event::kWrite)));

    // クライアントへのレスポンス送信
    // NOTE: クライアントConnectionは既にRepositoryに存在しているはず
    // AddConnectionActionは使わず、既存のConnectionを保持する
    // RegisterEventHandlerByFdActionを使用してfdで直接ハンドラーを登録
    actions.push_back(
        new RegisterEventHandlerByFdAction(clientFd_, Event::kWrite, new WriteResponseHandler(httpResponse))
    );

    return actions;
}

http::HttpStatusCode ReadCgiResponseHandler::determineStatusCode(const cgi::Response &response) {
    const cgi::Headers &cgiHeaders = response.getHeaders();

    const cgi::Headers::const_iterator it = cgiHeaders.find("Status");
    if (it == cgiHeaders.end()) {
        // "Status" ヘッダーが存在しない場合はデフォルトのステータスコードを返す
        return http::kStatusOk;
    }

    // "200 OK" のような形式から数値を抽出
    const std::string &statusStr = it->second;
    if (!statusStr.empty()) {
        std::istringstream iss(statusStr);
        int rawCode;
        if (iss >> rawCode) {
            const Option<http::HttpStatusCode> code = http::httpStatusCodeFromInt(rawCode);
            if (code.isSome()) {
                return code.unwrap();
            }
            LOG_WARNF("Invalid status code: %d", rawCode);
        }
    }

    // 有効な "Status" ヘッダーがないなら、成功扱いにする
    return http::kStatusOk;
}

Result<cgi::Response, error::AppError> ReadCgiResponseHandler::createCgiResponseFromBuffer(const std::string &buf) {
    const std::size_t headerEnd = buf.find("\n\n");
    if (headerEnd == std::string::npos) {
        // ヘッダーが不完全
        LOG_WARN("incomplete CGI headers");
        cgi::Headers headers;
        headers["Content-Type"] = "text/plain"; // デフォルト
        return cgi::Response::create(headers, Some(buf));
    }

    const std::string headerPart = buf.substr(0, headerEnd);
    const std::string bodyPart = buf.substr(headerEnd + 2);

    // CGIヘッダーをパース（RequestParser::parseHeaderFieldLineを使用）
    cgi::Headers headers;
    std::istringstream headerStream(headerPart);
    std::string line;
    while (std::getline(headerStream, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        const http::RequestParser::ParseHeaderFieldLineResult result = http::RequestParser::parseHeaderFieldLine(line);
        if (result.isOk()) {
            const http::RequestParser::HeaderField &field = result.unwrap();
            headers[field.first] = field.second;
        }
    }
    // Content-Typeが設定されていない場合はデフォルト値を設定
    if (headers.find("Content-Type") == headers.end()) {
        headers["Content-Type"] = "text/plain";
    }
    return cgi::Response::create(headers, Some(bodyPart));
}
