#ifndef SRC_LIB_HTTP_HANDLER_ROUTER_HPP
#define SRC_LIB_HTTP_HANDLER_ROUTER_HPP

#include "handler.hpp"
#include "matcher.hpp"
#include "config/config.hpp"
#include "utils/non_copyable.hpp"
#include "middleware/middleware.hpp"

namespace http {
    // TODO: 責務過多なのでリファクタしたい
    // IHandler * を持っているので、double free を避けるために NonCopyable にしている
    class Router : NonCopyable, public IHandler {
    public:
        Router();
        ~Router();

        Response serve(const Request &req);

        // 登録された handler は Router が delete する
        void on(HttpMethod method, const std::string &path, IHandler *handler);
        void on(const std::vector<HttpMethod> &methods, const std::string &path, IHandler *handler);
        // 登録された middleware は Router が delete する
        void use(IMiddleware *middleware);

    private:
        // (HttpMethod, Path) -> IHandler とするのが自然だが、Method Not Allowed を返すために必要なデータ構造にしている
        typedef std::string Path;
        typedef std::map<HttpMethod, IHandler *> MethodHandlerMap;
        typedef std::map<Path, MethodHandlerMap> HandlerMap;
        HandlerMap handlers_;

        typedef std::vector<IMiddleware *> MiddlewareList;
        MiddlewareList middlewares_;

        /**
         * handlersChain_ は以下のような構造になる。矢印は参照関係。
         * back() の serve を呼ぶことで、FILO で middleware が実行される。
         * (middleware がない場合は、InternalRouter が呼ばれる)
         *
         * [0]               [1]                [2]                handlersChain_ の index
         * InternalRouter <- ChainedHandler1 <- ChainedHandler2 <- ...
         * middlewares[0] <┘ middlewares[1]  <┘
         */
        std::vector<IHandler *> handlersChain_;

        class InternalRouter : public IHandler {
        public:
            explicit InternalRouter(HandlerMap &handlers);
            Response serve(const Request &req);

        private:
            HandlerMap &handlers_;
            Matcher<Path> createMatcher() const;
        };

        class ChainHandler : public IHandler {
        public:
            ChainHandler(IMiddleware &middleware, IHandler &next);
            Response serve(const Request &req);

        private:
            IMiddleware &middleware_;
            IHandler &next_;
        };
    };
}

#endif
