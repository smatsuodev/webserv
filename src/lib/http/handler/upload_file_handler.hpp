#ifndef SRC_LIB_HTTP_UPLOAD_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_UPLOAD_FILE_HANDLER_HPP
#include "handler.hpp"
#include "config/config.hpp"

namespace http {
    class UploadFileHandler : public IHandler {
    public:
        explicit UploadFileHandler(const config::LocationContext::DocumentRootConfig &config);
        Response serve(const Request &req);

    private:
        config::LocationContext::DocumentRootConfig docConfig_;
    };
}

#endif
