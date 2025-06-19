#include "read_cgi_response_handler.hpp"
#include "core/action/action.hpp"
#include "core/handler/write_response_handler.hpp"
#include "utils/logger.hpp"
#include "cgi/factory.hpp"
#include <cerrno>
#include <unistd.h>
#include <sys/wait.h>

ReadCgiResponseHandler::ReadCgiResponseHandler(int cgiSocketFd, pid_t cgiPid)
    : cgiSocketFd_(cgiSocketFd), cgiPid_(cgiPid), isResponseComplete_(false) {}

ReadCgiResponseHandler::~ReadCgiResponseHandler() {}

bool ReadCgiResponseHandler::checkCgiProcessStatus() {
    int status;
    pid_t result = waitpid(cgiPid_, &status, WNOHANG);

    if (result == -1) {
        LOG_WARN("waitpid failed");
        return true; // プロセスが終了したとみなす
    } else if (result == 0) {
        // プロセスがまだ実行中
        return false;
    } else {
        // プロセスが終了した
        if (WIFEXITED(status)) {
            LOG_DEBUG("CGI process exited with code: " + std::to_string(WEXITSTATUS(status)));
        } else if (WIFSIGNALED(status)) {
            LOG_DEBUG("CGI process terminated by signal: " + std::to_string(WTERMSIG(status)));
        }
        return true;
    }
}

IEventHandler::InvokeResult ReadCgiResponseHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start ReadCgiResponseHandler::invoke");

    // CGIプロセスの状態を確認
    bool processEnded = checkCgiProcessStatus();

    // CGIからのレスポンスを読み取り
    char buffer[4096];
    errno = 0;
    ssize_t bytesRead = read(cgiSocketFd_, buffer, sizeof(buffer));

    if (bytesRead == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 非ブロッキングI/Oで読み取り不可の場合
            if (processEnded) {
                // プロセスが終了している場合、レスポンス完了
                isResponseComplete_ = true;
            } else {
                // まだ読み取るデータがある可能性がある
                LOG_DEBUG("read would block, retry later");
                return Ok(std::vector<IAction *>());
            }
        } else {
            LOG_WARN("failed to read CGI response");
            return Err(error::kIOUnknown);
        }
    } else if (bytesRead == 0) {
        // EOF (プロセスが標準出力を閉じた)
        LOG_DEBUG("CGI response EOF");
        isResponseComplete_ = true;
    } else {
        // データを読み取った
        responseBuffer_.append(buffer, bytesRead);
        LOG_DEBUG("CGI response bytes read: " + std::to_string(bytesRead));

        // プロセスが終了している場合、これ以上データはない
        if (processEnded) {
            isResponseComplete_ = true;
        }
    }

    if (isResponseComplete_) {
        // レスポンスの解析とHTTPレスポンスの生成
        LOG_DEBUG("CGI response completed, parsing...");

        // CGIレスポンスの解析（簡単な実装）
        cgi::Headers headers;
        std::string body;

        // ヘッダーとボディを分離
        size_t headerEnd = responseBuffer_.find("\r\n\r\n");
        if (headerEnd == std::string::npos) {
            headerEnd = responseBuffer_.find("\n\n");
            if (headerEnd == std::string::npos) {
                // ヘッダーのみの場合
                body = responseBuffer_;
            } else {
                body = responseBuffer_.substr(headerEnd + 2);
            }
        } else {
            body = responseBuffer_.substr(headerEnd + 4);
        }

        // 最低限Content-Typeヘッダーを追加
        headers["Content-Type"] = "text/html";

        Result<cgi::Response, error::AppError> cgiResponseResult = cgi::Response::create(headers, Some(body));
        if (cgiResponseResult.isErr()) {
            LOG_WARN("failed to create CGI response");
            return Err(cgiResponseResult.unwrapErr());
        }

        const cgi::Response &cgiResponse = cgiResponseResult.unwrap();
        Result<http::Response, error::AppError> httpResponseResult = cgi::HttpResponseFactory::create(cgiResponse);
        if (httpResponseResult.isErr()) {
            LOG_WARN("failed to create HTTP response from CGI response");
            return Err(httpResponseResult.unwrapErr());
        }

        const http::Response &httpResponse = httpResponseResult.unwrap();

        // WriteResponseHandlerに切り替えてレスポンスを送信
        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(ctx.getEvent()));
        actions.push_back(
            new RegisterEventHandlerAction(ctx.getConnection().unwrap(), new WriteResponseHandler(httpResponse))
        );

        return Ok(actions);
    }

    // まだレスポンスが完了していない場合、継続
    return Ok(std::vector<IAction *>());
}
