#ifndef SRC_LIB_HTTP_DELETE_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_DELETE_FILE_HANDLER_HPP
#include "handler.hpp"
#include "delete_file_handler.hpp"
#include "config/config.hpp"

namespace http {
    class DeleteFileHandler : public IHandler {
    public:
        explicit DeleteFileHandler(const config::LocationContext::DocumentRootConfig &docRootConfig);
        Either<IAction *, Response> serve(const RequestContext &ctx);

    private:
        config::LocationContext::DocumentRootConfig docRootConfig_;

        Response serveInternal(const Request &req) const;
    };
}

#endif
