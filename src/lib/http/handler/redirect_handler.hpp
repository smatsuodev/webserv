#ifndef SRC_LIB_HTTP_HANDLER_REDIRECT_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_REDIRECT_HANDLER_HPP

#include "handler.hpp"

namespace http {
    class RedirectHandler : public IHandler {
    public:
        explicit RedirectHandler(const std::string &destination);
        Either<IAction *, Response> serve(const Request &req);

    private:
        std::string destination_;

        Response serveInternal(const Request &req) const;
    };
}

#endif
