#ifndef SRC_LIB_HTTP_HANDLER_MIDDLEWARE_MIDDLEWARE_HPP
#define SRC_LIB_HTTP_HANDLER_MIDDLEWARE_MIDDLEWARE_HPP

#include "../handler.hpp"

namespace http {
    class IMiddleware {
    public:
        virtual ~IMiddleware() {}
        virtual Either<IAction *, Response> intercept(const RequestContext &ctx, IHandler &next) = 0;
    };
}

#endif
