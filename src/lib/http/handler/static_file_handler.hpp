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
        static std::string getNextLink(const std::string &currentDirPath, const std::string &entryName, bool isDir);
        static Result<std::string, HttpStatusCode> makeDirectoryListingHtml(const std::string &path);
        static Response directoryListing(const std::string &path);
        config::LocationContext::DocumentRootConfig docRootConfig_;
    };
} // http


#endif
