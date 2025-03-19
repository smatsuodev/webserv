#ifndef SRC_LIB_HTTP_HANDLER_SOME_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_SOME_HANDLER_HPP

#include "handler.hpp"
#include "cgi/factory.hpp"
#include "cgi/request.hpp"
#include "core/action.hpp"
#include "event/event_handler.hpp"
#include "utils/types/either.hpp"
#include "utils/types/try.hpp"

class IAction;

class WriteCgiRequestHandler : public IEventHandler {};

namespace http {
    // connection を RunCgiHandler のコンストラクタに渡す方式だと、インスタンスを作るタイミングが限られて困る
    struct RequestContext {
        Ref<Request> req;
        Ref<Connection> conn;
    };

    class IHandlerNew {
    public:
        // 引数も戻り値も従来の http::IHandler と異なる
        virtual Either<IAction *, Response> serve(const RequestContext &ctx) = 0;
    };

    class RunCgiHandler : public IHandlerNew {
        // コンストラクタでもらう
        std::string serverName_;
        IHandler *fallbackHandler_;

        // Location の情報が必要? 必要ならコンストラクタで好きに受け取れそう
        std::string getScriptName();
        std::string getPathInfo();

    public:
        Either<IAction *, Response> serve(const RequestContext &ctx) override {
            int pipeIn;
            const cgi::RequestFactory::Parameter param = {
                ctx.req,
                ctx.conn.get().getForeignAddress(),
                serverName_,
                "port", // localAddress の port ではだめ? 分からない
                this->getScriptName(),
                this->getPathInfo()
            };
            const cgi::Request cgiReq = cgi::RequestFactory::create(param).unwrap();

            return Left(new RegisterEventHandlerAction(conn_, new WriteCgiRequestHandler(pipeIn, cgiReq)));
        }
    };
}

#endif
