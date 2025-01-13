#ifndef SRC_LIB_HTTP_HANDLER_MIDDLEWARE_ERROR_PAGE_HPP
#define SRC_LIB_HTTP_HANDLER_MIDDLEWARE_ERROR_PAGE_HPP

#include "./middleware.hpp"

namespace http {
    // TODO: 設定を受け取る
    class ErrorPage : public IMiddleware {
    public:
        Response intercept(const Request &req, IHandler &next);
    };
}

#endif
