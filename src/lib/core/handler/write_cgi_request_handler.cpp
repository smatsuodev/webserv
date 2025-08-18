#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include "write_cgi_request_handler.hpp"
#include "../action/action.hpp"
#include "../../event/event.hpp"
#include "../../transport/connection.hpp"
#include "../../utils/logger.hpp"

WriteCgiRequestBodyHandler::WriteCgiRequestBodyHandler(const std::string &cgiBody) : body_(cgiBody), bytesWritten_(0) {}

WriteCgiRequestBodyHandler::~WriteCgiRequestBodyHandler() {}

IEventHandler::InvokeResult WriteCgiRequestBodyHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start WriteCgiRequestBodyHandler::invoke");

    Connection &conn = ctx.getConnection().unwrap();
    const size_t bytesToWrite = body_.size() - bytesWritten_;

    if (bytesToWrite == 0) {
        // 書き込み完了
        LOG_DEBUG("CGI request body fully written");
        shutdown(conn.getFd(), SHUT_WR);

        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kWrite)));
        actions.push_back(new UnregisterEventHandlerAction(conn, Event::kWrite));
        return Ok(actions);
    }

    errno = 0;
    const ssize_t bytesWritten = write(conn.getFd(), body_.c_str() + bytesWritten_, bytesToWrite);
    if (bytesWritten == -1 && errno == EPIPE) {
        // 子が読み込みを待たずに終了すると、EPIPE が起こり得る
        LOG_DEBUG("write failed with EPIPE, likely because the CGI process has terminated");
        shutdown(conn.getFd(), SHUT_WR);
        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kWrite)));
        actions.push_back(new UnregisterEventHandlerAction(conn, Event::kWrite));
        return Ok(actions);
    }

    if (bytesWritten == -1) {
        LOG_WARN("failed to write CGI request body");
        // エラー時のクリーンアップ
        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kRead)));
        actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kWrite)));
        actions.push_back(new UnregisterEventHandlerAction(conn, Event::kRead));
        actions.push_back(new UnregisterEventHandlerAction(conn, Event::kWrite));
        actions.push_back(new RemoveConnectionAction(conn));
        return Ok(actions);
    }

    bytesWritten_ += bytesWritten;
    LOG_DEBUGF("CGI request body: %zu/%zu bytes written", bytesWritten_, body_.size());

    if (bytesWritten_ == body_.size()) {
        // 書き込み完了
        LOG_DEBUG("CGI request body fully written");
        shutdown(conn.getFd(), SHUT_WR);

        std::vector<IAction *> actions;
        actions.push_back(new UnregisterEventAction(Event(conn.getFd(), Event::kWrite)));
        actions.push_back(new UnregisterEventHandlerAction(conn, Event::kWrite));
        return Ok(actions);
    }

    // まだ書き込むデータが残っている
    return Err(error::kRecoverable);
}
