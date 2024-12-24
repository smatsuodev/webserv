#include "accept_handler.hpp"

#include "core/server.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

AcceptHandler::AcceptHandler(EventNotifier &notifier, Listener &listener) : notifier_(notifier), listener_(listener) {}

Result<void, error::AppError> AcceptHandler::invoke(Context &ctx) {
    const Listener::AcceptConnectionResult result = listener_.acceptConnection();
    if (result.isErr()) {
        LOG_WARNF("failed to accept connection: %s", result.unwrapErr().c_str());
        return Err(error::kUnknown);
    }

    Connection *newConnection = result.unwrap();
    const Event eventToRegister = Event(newConnection->getFd());

    // 各種登録
    notifier_.registerEvent(eventToRegister);
    ctx.getServer().addConnection(newConnection);
    // TODO: 適切なオブジェクトを登録
    ctx.getServer().registerEventHandler(newConnection->getFd(), NULL);

    return Ok();
}
