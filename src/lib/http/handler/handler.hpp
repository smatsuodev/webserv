#ifndef SRC_LIB_HTTP_HANDLER__HPP
#define SRC_LIB_HTTP_HANDLER_HPP

#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"

namespace http {
    class IHandler {
    public:
        virtual ~IHandler();
        virtual Result<Response, error::AppError> serve(Request &req) = 0;
    };
}

#endif
