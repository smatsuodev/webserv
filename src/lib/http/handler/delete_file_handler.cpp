#include "delete_file_handler.hpp"
#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"
#include <sys/stat.h>
#include <cerrno>
#include <cstdio>

namespace http {
    DeleteFileHandler::DeleteFileHandler(const config::LocationContext::DocumentRootConfig &docRootConfig)
        : docRootConfig_(docRootConfig) {}

    Either<IAction *, Response> DeleteFileHandler::serve(const RequestContext &ctx) {
        return Right(this->serveInternal(ctx.getRequest()));
    }

    Response DeleteFileHandler::serveInternal(const Request &req) const {
        const std::string path = docRootConfig_.getRoot() + '/' + req.getRequestTarget();
        LOG_DEBUGF("request target: %s", req.getRequestTarget().c_str());

        struct stat buf = {};
        if (stat(path.c_str(), &buf) == -1) {
            if (errno == ENOENT) {
                LOG_DEBUGF("file does not exist: %s", path.c_str());
                return ResponseBuilder().status(kStatusNotFound).build();
            }
            LOG_DEBUGF("failed to stat file: %s", path.c_str());
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (!S_ISREG(buf.st_mode)) {
            LOG_DEBUGF("is not a regular file: %s", path.c_str());
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        if (std::remove(path.c_str())) {
            LOG_DEBUGF("failed to delete file: %s", path.c_str());
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        LOG_DEBUGF("file deleted: %s", path.c_str());
        return ResponseBuilder().status(kStatusNoContent).build();
    }
}
