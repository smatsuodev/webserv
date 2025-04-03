#ifndef SRC_LIB_HTTP_HANDLER_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_HANDLER_HPP

#include "../response/response.hpp"
#include "../request/request.hpp"
#include "event/event_handler.hpp"
#include "utils/types/either.hpp"

namespace http {
    class RequestContext {
    public:
        RequestContext(const Request &request, const Ref<const Connection> &connection)
            : request_(request), connection_(connection) {}

        const Request &getRequest() const {
            return request_;
        }

        const Ref<const Connection> &getConnection() const {
            return connection_;
        }

    private:
        Ref<const Request> request_;
        Ref<const Connection> connection_;
    };

    class IHandler {
    public:
        virtual ~IHandler() {}
        /**
         * http handler 自身ではレスポンスを作成できず、別の処理を上流に通知する場合に IAction * を返す
         * この課題では CGI の実行を行うため、IAction * を返す
         * (CGI はいくらでもブロッキングしうるので、epoll などで IO の通知を待つ必要がある)
         */
        virtual Either<IAction *, Response> serve(const RequestContext &ctx) = 0;
    };
}

#endif
