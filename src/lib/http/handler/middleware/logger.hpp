#ifndef SRC_LIB_HTTP_HANDLER_MIDDLEWARE_LOGGER_HPP
#define SRC_LIB_HTTP_HANDLER_MIDDLEWARE_LOGGER_HPP

#include "./middleware.hpp"

namespace http {
    class Logger : public IMiddleware {
    public:
        Either<IAction *, Response> intercept(const RequestContext &ctx, IHandler &next);
    };
}

#endif
