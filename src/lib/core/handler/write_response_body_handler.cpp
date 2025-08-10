#include "write_response_body_handler.hpp"
#include "core/action/action.hpp"
#include "utils/logger.hpp"
#include <cerrno>
#include <unistd.h>

WriteResponseHandler::WriteResponseHandler(const http::Response &response)
    : responseMessage_(response.toString()), totalBytesWritten_(0) {}

IEventHandler::InvokeResult WriteResponseHandler::invoke(const Context &ctx) {
    LOG_DEBUG("start WriteResponseHandler::invoke");

    Connection &conn = ctx.getConnection().unwrap();
    const size_t bytesToWrite = responseMessage_.size() - totalBytesWritten_;

    errno = 0;
    const ssize_t bytesWritten = write(conn.getFd(), responseMessage_.c_str() + totalBytesWritten_, bytesToWrite);
    if (bytesWritten == -1) {
        LOG_WARN("failed to write response");
        return Err(error::kIOUnknown);
    }
    totalBytesWritten_ += bytesToWrite;

    LOG_DEBUG("response written");

    std::vector<IAction *> actions;
    actions.push_back(new UnregisterEventAction(ctx.getEvent()));
    actions.push_back(new UnregisterEventHandlerAction(conn, Event::kRead));
    actions.push_back(new UnregisterEventHandlerAction(conn, Event::kWrite));
    actions.push_back(new RemoveConnectionAction(ctx.getConnection().unwrap()));

    return Ok(actions);
}
