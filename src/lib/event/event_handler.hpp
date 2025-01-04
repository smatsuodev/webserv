#ifndef SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP

#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "event.hpp"
#include "transport/connection.hpp"
#include "utils/ref.hpp"

#include <vector>

class ActionContext;

// イベントハンドラーに呼び出された文脈を提供する
class Context {
public:
    Context(const Option<Ref<Connection> > &conn, const Event &event);

    Option<Ref<Connection> > getConnection() const;
    const Event &getEvent() const;

private:
    // accept 前は connection は存在しない
    Option<Ref<Connection> > conn_;
    Event event_;
};

// IEventHandler が返す Command オブジェクト
class IAction {
public:
    virtual ~IAction();
    // NOTE: core の ActionContext に依存していて、設計的に微妙?
    virtual void execute(ActionContext &ctx) = 0;
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
