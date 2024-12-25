#include "accept_handler.hpp"

#include "echo_handler.hpp"
#include "core/server.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

AcceptHandler::AcceptHandler(EventNotifier &notifier, Listener &listener) : notifier_(notifier), listener_(listener) {}

Result<void, error::AppError> AcceptHandler::invoke(Context &ctx) {
    // TODO: accept がブロックしたら?
    const Listener::AcceptConnectionResult result = listener_.acceptConnection();
    if (result.isErr()) {
        LOG_WARNF("failed to accept connection: %s", result.unwrapErr().c_str());
        return Err(error::kUnknown);
    }

    Connection *newConnection = result.unwrap();
    const Event eventToRegister = Event(newConnection->getFd());

    // 各種登録
    /**
     * TODO: newConnection を戻り値で返したい
     * accept する役割は既に終えているので、これらの処理は Server でやりたい
     */
    notifier_.registerEvent(eventToRegister);
    ctx.getServer().addConnection(newConnection);
    ctx.getServer().registerEventHandler(newConnection->getFd(), new EchoHandler());

    return Ok();
}
