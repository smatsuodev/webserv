#ifndef SRC_LIB_HTTP_HANDLER_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_HANDLER_HPP

#include "utils/types/result.hpp"
#include "../response/response.hpp"
#include "../request/request.hpp"
#include "utils/types/error.hpp"

namespace http {
    class IHandler {
    public:
        virtual ~IHandler();
        virtual Result<Response, error::AppError> serve(const Request &req) = 0;
    };

    class Handler : public IHandler {
    public:
        Result<Response, error::AppError> serve(const Request &req);
    };
}

#endif
