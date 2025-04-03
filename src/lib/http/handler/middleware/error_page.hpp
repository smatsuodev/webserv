#ifndef SRC_LIB_HTTP_HANDLER_MIDDLEWARE_ERROR_PAGE_HPP
#define SRC_LIB_HTTP_HANDLER_MIDDLEWARE_ERROR_PAGE_HPP

#include "./middleware.hpp"
#include "config/config.hpp"

namespace http {
    class ErrorPage : public IMiddleware {
    public:
        explicit ErrorPage(const config::ServerContext::ErrorPageMap &errorPage);
        Either<IAction *, Response> intercept(const RequestContext &ctx, IHandler &next);

    private:
        config::ServerContext::ErrorPageMap errorPage_;
    };
}

#endif
