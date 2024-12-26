#ifndef SRC_LIB_HTTP_HANDLER_REQUEST_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_REQUEST_HANDLER_HPP

#include "http/request/request.hpp"
#include "http/response/response.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"

namespace http {
    class IRequestHandler {
    public:
        virtual ~IRequestHandler();
        virtual Result<Response, error::AppError> serve(Request &req) = 0;
    };
}

#endif
