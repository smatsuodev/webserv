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
        virtual Either<IAction *, Response> serve(const Request &req) = 0;
    };
}

#endif
