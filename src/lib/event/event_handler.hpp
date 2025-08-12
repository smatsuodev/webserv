#ifndef SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_EVENT_HANDLER_HPP

#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include "utils/types/option.hpp"
#include "event.hpp"
#include "transport/connection.hpp"
#include "utils/ref.hpp"
#include <vector>

// core のクラスの前方宣言
class ActionContext;
class VirtualServerResolver;

class IVirtualServerResolverFactory {
public:
    virtual ~IVirtualServerResolverFactory();
    virtual VirtualServerResolver create(const Ref<Connection> &conn) = 0;
};

// イベントハンドラーに呼び出された文脈を提供する
class Context {
public:
    /**
     * NOTE: Server は HTTP レイヤーの情報を知らないため、VirtualServer を解決するためのオブジェクトを渡す
     * 構築が面倒なので、VirtualServerResolver は Ref にしてない
     */
    explicit Context(
        const Event &event, const Option<Ref<Connection> > &conn, IVirtualServerResolverFactory &resolverFactory
    );

    Option<Ref<Connection> > getConnection() const;
    const Event &getEvent() const;
    // accept 前は Connection がないので None になる
    Option<VirtualServerResolver> getResolver() const;

    // Event, Connection を差し替えた新しい Context を返す
    Context withNewValues(const Event &event, const Option<Ref<Connection> > &conn) const;

private:
    Event event_;
    // accept 前は connection は存在しない
    Option<Ref<Connection> > conn_;
    IVirtualServerResolverFactory &resolverFactory_;
};

// IEventHandler が返す Command オブジェクト
class IAction {
public:
    virtual ~IAction() {}
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

    struct ErrorHandleResult {
        bool shouldFallback;
        std::vector<IAction *> actions;

        ErrorHandleResult() : shouldFallback(true) {}

        ErrorHandleResult(bool fallback, const std::vector<IAction *> &acts)
            : shouldFallback(fallback), actions(acts) {}
    };

    virtual ErrorHandleResult onErrorEvent(const Context &, const Event &event) {
        return ErrorHandleResult();
    }
};

#endif
