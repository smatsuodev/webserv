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

ReadCgiResponseHandler::ReadCgiResponseHandler(int clientFd, pid_t childPid)
    : responseBuffer_(""), headersParsed_(false), cgiResponse_(None), clientFd_(clientFd), childPid_(childPid) {}

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

    if (bytesRead == 0) {
        // EOF - CGIプロセスが終了
        LOG_DEBUG("CGI process finished");

        // CGIレスポンスをパース
        if (!headersParsed_) {
            // ヘッダーをパース
            const size_t headerEnd = responseBuffer_.find("\r\n\r\n");
            if (headerEnd != std::string::npos) {
                const std::string headerPart = responseBuffer_.substr(0, headerEnd);
                const std::string bodyPart = responseBuffer_.substr(headerEnd + 4);

                // CGIヘッダーをパース（RequestParser::parseHeaderFieldLineを使用）
                cgi::Headers headers;
                std::istringstream headerStream(headerPart);
                std::string line;
                while (std::getline(headerStream, line)) {
                    if (!line.empty() && line.back() == '\r') {
                        line.pop_back();
                    }
                    const http::RequestParser::ParseHeaderFieldLineResult result =
                        http::RequestParser::parseHeaderFieldLine(line);
                    if (result.isOk()) {
                        const http::RequestParser::HeaderField &field = result.unwrap();
                        headers[field.first] = field.second;
                    }
                }
                // Content-Typeが設定されていない場合はデフォルト値を設定
                if (headers.find("Content-Type") == headers.end()) {
                    headers["Content-Type"] = "text/plain";
                }
                const Result<cgi::Response, error::AppError> result = cgi::Response::create(headers, Some(bodyPart));
                if (result.isOk()) {
                    cgiResponse_ = Some(result.unwrap());
                }
            } else {
                // ヘッダーが不完全
                LOG_WARN("Incomplete CGI headers");
                cgi::Headers headers;
                headers["Content-Type"] = "text/plain"; // デフォルト
                const Result<cgi::Response, error::AppError> result =
                    cgi::Response::create(headers, Some(responseBuffer_));
                if (result.isOk()) {
                    cgiResponse_ = Some(result.unwrap());
                }
            }
        }

        // HTTPレスポンスを生成
        http::ResponseBuilder builder;

        // CGIヘッダーをHTTPヘッダーに変換
        http::HttpStatusCode statusCode = http::kStatusOk;
        if (cgiResponse_.isSome()) {
            const cgi::Response &response = cgiResponse_.unwrap();
            const cgi::Headers &cgiHeaders = response.getHeaders();

            // Statusヘッダーの処理
            auto statusIt = cgiHeaders.find("Status");
            if (statusIt != cgiHeaders.end()) {
                // "200 OK" のような形式から数値を抽出
                const std::string &statusStr = statusIt->second;
                if (!statusStr.empty()) {
                    std::istringstream iss(statusStr);
                    int code;
                    if (iss >> code) {
                        statusCode = static_cast<http::HttpStatusCode>(code);
                    }
                }
            }

            // その他のヘッダーをHTTPヘッダーに設定
            for (const auto &header : cgiHeaders) {
                // Statusヘッダーは既に処理済みなのでスキップ
                if (header.first != "Status") {
                    builder.header(header.first, header.second);
                }
            }
        }

        builder.status(statusCode);

        // ボディを設定
        if (cgiResponse_.isSome()) {
            const cgi::Response &response = cgiResponse_.unwrap();
            if (response.getBody().isSome()) {
                builder.text(response.getBody().unwrap());
            }
        }

        const http::Response httpResponse = builder.build();
        const std::string responseMessage = httpResponse.toString();

        // 子プロセスの回収
        int status;
        waitpid(childPid_, &status, WNOHANG);

        // クライアントへレスポンスを送信するアクションを作成
        std::vector<IAction *> actions;

        // CGIソケットのクリーンアップ
        actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kRead)));
        actions.push_back(new UnregisterEventHandlerAction(conn, Event::kRead));
        actions.push_back(new RemoveConnectionAction(conn));

        // クライアントへのレスポンス送信
        // clientFd_に対してWriteイベントとハンドラを登録
        actions.push_back(new RegisterEventAction(Event(clientFd_, Event::kWrite)));

        // クライアントConnectionを仮想的に作成（FIXME: 本来はConnectionRepositoryから取得すべき）
        const Address dummyLocal("127.0.0.1", 0);
        const Address dummyForeign("127.0.0.1", 0);
        Connection *clientConn = new Connection(clientFd_, dummyLocal, dummyForeign);

        actions.push_back(new AddConnectionAction(clientConn));
        actions.push_back(
            new RegisterEventHandlerAction(*clientConn, Event::kWrite, new WriteResponseHandler(httpResponse))
        );

        return Ok(actions);
    }

    // データを蓄積
    responseBuffer_.append(buffer, bytesRead);
    LOG_DEBUGF("CGI response: %zd bytes read, total: %zu", bytesRead, responseBuffer_.size());

    // ヘッダーの終わりを探す
    if (!headersParsed_) {
        const size_t headerEnd = responseBuffer_.find("\r\n\r\n");
        if (headerEnd != std::string::npos) {
            headersParsed_ = true;
            LOG_DEBUG("CGI headers parsed");

            const std::string headerPart = responseBuffer_.substr(0, headerEnd);
            const std::string bodyPart = responseBuffer_.substr(headerEnd + 4);

            // ヘッダーをパース（RequestParser::parseHeaderFieldLineを使用）
            cgi::Headers headers;
            std::istringstream headerStream(headerPart);
            std::string line;
            while (std::getline(headerStream, line)) {
                if (!line.empty() && line.back() == '\n') {
                    line.pop_back();
                }
                // RequestParser::parseHeaderFieldLineを使用してヘッダーをパース
                const http::RequestParser::ParseHeaderFieldLineResult result =
                    http::RequestParser::parseHeaderFieldLine(line);
                if (result.isOk()) {
                    const http::RequestParser::HeaderField &field = result.unwrap();
                    headers[field.first] = field.second;
                }
            }
        }
    } else {
        // ボディを追加
        if (cgiResponse_.isSome()) {
            const cgi::Response &currentResponse = cgiResponse_.unwrap();
            const Option<std::string> currentBody = currentResponse.getBody();
            if (currentBody.isSome()) {
                std::string newBody = currentBody.unwrap();
                newBody.append(buffer, bytesRead);
                const Result<cgi::Response, error::AppError> updateResult =
                    cgi::Response::create(currentResponse.getHeaders(), Some(newBody));
                if (updateResult.isOk()) {
                    cgiResponse_ = Some(updateResult.unwrap());
                }
            }
        }
    }

    // まだデータが来る可能性がある
    return Err(error::kRecoverable);
}
