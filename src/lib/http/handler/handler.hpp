#ifndef SRC_LIB_HTTP_HANDLER_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_HANDLER_HPP

#include "../response/response.hpp"
#include "../request/request.hpp"

namespace http {
    class IHandler {
    public:
        virtual ~IHandler() {}
        virtual Response serve(const Request &req) = 0;
    };
}

#endif
