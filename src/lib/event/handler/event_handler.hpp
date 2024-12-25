#ifndef SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP

#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "event/event.hpp"

#include <vector>

class IAction;
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
    // Connection は参照で受け取りたいが、Option<T &> が無理
    Context(const Option<Connection *> &conn, const Event &event);

    Option<Connection *> getConnection() const;
    const Event &getEvent() const;

private:
    // accept 前は connection は存在しない
    Option<Connection *> conn_;
    Event event_;
};

class IEventHandler {
public:
    virtual ~IEventHandler();

    /**
     * NOTE: std::vector のコピーを防ぐために、引数で参照をもらう?
     * RVO が効けば問題にはならない。C++17 ですら保証はされていないが。
     * https://cpprefjp.github.io/lang/cpp17/guaranteed_copy_elision.html
     */
    /**
     * Connection の管理は Server の責務
     * Server に実行してほしい処理を IAction オブジェクトとして返す (Command パターン)
     */
    typedef Result<std::vector<IAction *>, error::AppError> InvokeResult;
    virtual InvokeResult invoke(const Context &ctx) = 0;
};

#endif
