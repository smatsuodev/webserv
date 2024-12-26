#ifndef SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP

#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "event.hpp"
#include "transport/connection.hpp"

#include <vector>

class Server;
class ServerState;

// イベントハンドラーに呼び出された文脈を提供する
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

// IEventHandler が返す Command オブジェクト
class IAction {
public:
    virtual ~IAction();
    virtual void execute(ServerState &state) = 0;
};

// IEventHandler は Context を受けとり、IAction を返す
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
     *
     * Server は順番通り実行すること
     */
    typedef Result<std::vector<IAction *>, error::AppError> InvokeResult;
    virtual InvokeResult invoke(const Context &ctx) = 0;
};

#endif
