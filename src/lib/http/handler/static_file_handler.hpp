#ifndef SRC_LIB_HTTP_STATIC_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_STATIC_FILE_HANDLER_HPP

#include "handler.hpp"
#include "config/config.hpp"

namespace http {
    class StaticFileHandler : public IHandler {
    public:
        explicit StaticFileHandler(const config::LocationContext::DocumentRootConfig &docRootConfig);
        static std::string getNextLink(const std::string &newPath, struct dirent *dp);
        static Response directoryListing(const std::string &path);
        virtual Response serve(const Request &req);

    private:
        config::LocationContext::DocumentRootConfig docRootConfig_;
    };
} // http


#endif
