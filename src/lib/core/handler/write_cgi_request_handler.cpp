#include "write_cgi_request_handler.hpp"
#include "core/action/action.hpp"
#include "utils/logger.hpp"
#include <cerrno>
#include <unistd.h>

WriteCgiRequestHandler::WriteCgiRequestHandler(int cgiSocketFd, const cgi::Request &cgiRequest)
    : cgiSocketFd_(cgiSocketFd), totalBytesWritten_(0) {
    // CGIリクエストのボディ部分を取得
    const Option<std::string> &body = cgiRequest.getBody();
    if (body.isSome()) {
        requestData_ = body.unwrap();
    }
}

WriteCgiRequestHandler::~WriteCgiRequestHandler() {}

IEventHandler::InvokeResult WriteCgiRequestHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start WriteCgiRequestHandler::invoke");

    if (requestData_.empty()) {
        // リクエストボディがない場合、書き込み完了
        LOG_DEBUG("no request body to write");
        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(ctx.getEvent()));
        return Ok(actions);
    }

    const size_t bytesToWrite = requestData_.size() - totalBytesWritten_;
    if (bytesToWrite == 0) {
        // 書き込み完了
        LOG_DEBUG("CGI request writing completed");
        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(ctx.getEvent()));
        return Ok(actions);
    }

    errno = 0;
    const ssize_t bytesWritten = write(cgiSocketFd_, requestData_.c_str() + totalBytesWritten_, bytesToWrite);
    if (bytesWritten == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 非ブロッキングI/Oで書き込み不可の場合、再試行
            LOG_DEBUG("write would block, retry later");
            return Ok(std::vector<IAction *>());
        }
        LOG_WARN("failed to write CGI request");
        return Err(error::kIOUnknown);
    }

    totalBytesWritten_ += bytesWritten;
    LOG_DEBUG("CGI request bytes written: " + std::to_string(bytesWritten));

    if (totalBytesWritten_ >= requestData_.size()) {
        // 書き込み完了
        LOG_DEBUG("CGI request writing completed");
        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(ctx.getEvent()));
        return Ok(actions);
    }

    // まだ書き込むデータがある場合、継続
    return Ok(std::vector<IAction *>());
}
