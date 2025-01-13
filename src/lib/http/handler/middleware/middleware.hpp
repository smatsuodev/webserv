#ifndef SRC_LIB_HTTP_HANDLER_MIDDLEWARE_MIDDLEWARE_HPP
#define SRC_LIB_HTTP_HANDLER_MIDDLEWARE_MIDDLEWARE_HPP

#include "../handler.hpp"

namespace http {
    class IMiddleware {
    public:
        virtual ~IMiddleware() {}
        virtual Response intercept(const Request &req, IHandler &next) = 0;
    };
}

#endif
