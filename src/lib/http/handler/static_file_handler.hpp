#ifndef SRC_LIB_HTTP_STATIC_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_STATIC_FILE_HANDLER_HPP

#include "handler.hpp"
#include "config/config.hpp"

namespace http {
    class StaticFileHandler : public IHandler {
    public:
        explicit StaticFileHandler(const config::LocationContext::DocumentRootConfig &docRootConfig);
        virtual Response serve(const Request &req);

    private:
        static Result<std::string, HttpStatusCode> makeDirectoryListingHtml(const std::string &root, const std::string &target);
        static Response directoryListing(const std::string &root, const std::string &target);
        config::LocationContext::DocumentRootConfig docRootConfig_;
    };
} // http


#endif
