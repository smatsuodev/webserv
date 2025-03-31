#ifndef SRC_LIB_HTTP_HANDLER_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_HANDLER_HPP

#include "../response/response.hpp"
#include "../request/request.hpp"
#include "event/event_handler.hpp"
#include "utils/types/either.hpp"

namespace http {
    class IHandler {
    public:
        virtual ~IHandler() {}
        /**
         * http handler 自身ではレスポンスを作成できず、別の処理を上流に通知する場合に IAction * を返す
         * この課題では CGI の実行を行うため、IAction * を返す
         * (CGI はいくらでもブロッキングしうるので、epoll などで IO の通知を待つ必要がある)
         */
        virtual Either<IAction *, Response> serve(const Request &req) = 0;
    };
}

#endif
