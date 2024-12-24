#ifndef SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP

#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "event/event.hpp"

class Server;
class Connection;

// イベントハンドラーに呼び出された文脈を提供する
/**
 * EventNotifier への参照は意図的に含めていない
 * なぜなら、"呼び出された文脈" には関係ないから
 * また、EventNotifier は常に固定なので、毎回渡し直す必要がない
 * (Connection や Event は状態遷移がある)
 *
 * 個々のハンドラーの実装に必要なものはコンストラクタで注入する
 */
// Connection がコピー禁止なので、Context もコピー禁止
class Context {
public:
    Context(Server &server, const Option<Connection &> &conn, const Event &event);

    const Server &getServer() const;
    Option<Connection &> getConnection() const;
    const Event &getEvent() const;

private:
    // どのサーバーの、どのコネクションの、何のイベントなのかを特定する必要がある
    Server &server_;
    // accept 前は connection は存在しない
    Option<Connection &> conn_;
    Event event_;
};

class IEventHandler {
public:
    virtual ~IEventHandler();
    virtual Result<void, error::AppError> invoke(Context &ctx) = 0;
};

#endif
