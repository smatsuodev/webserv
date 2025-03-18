#ifndef SRC_LIB_HTTP_CGI_HANDLER_HPP
#define SRC_LIB_HTTP_CGI_HANDLER_HPP
#include "handler.hpp"
#include "matcher.hpp"
#include "config/config.hpp"

namespace http {
    class CgiHandler : public IHandler {
    private:
        typedef std::string CgiExtension;
        typedef std::vector<CgiExtension> CgiExtensions;

        config::LocationContext::DocumentRootConfig docRootConfig_;
        IHandler *next_;
        Matcher<CgiExtension> createMatcher() const;

    public:
        explicit CgiHandler(const config::LocationContext::DocumentRootConfig &docRootConfig, IHandler *next);
        Response serve(const Request &req);
    };
}

#endif
