#include "handler.hpp"
#include "utils/logger.hpp"
#include "http/response/response_builder.hpp"
#include <fstream>
#include <sys/stat.h>
#include "static_file_handler.hpp"

namespace http {
    Response StaticFileHandler::serve(const Request &req) {
        LOG_DEBUGF("request target: %s", req.getRequestTarget().c_str());
        const std::string path = req.getRequestTarget().substr(1);

        if (path.empty()) {
            LOG_DEBUGF("invalid request target: %s", req.getRequestTarget().c_str());
            return ResponseBuilder().status(kStatusNotFound).build();
        }

        struct stat buf = {};
        if (stat(path.c_str(), &buf) == -1) {
            if (errno == ENOENT) {
                LOG_DEBUGF("file does not exist: %s", path.c_str());
                return ResponseBuilder().status(kStatusNotFound).build();
            }
            LOG_DEBUGF("failed to stat file: %s", path.c_str());
            return ResponseBuilder().status(kStatusInternalServerError).build();
        }

        if (S_ISDIR(buf.st_mode)) {
            LOG_DEBUGF("is a directory: %s", path.c_str());
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        if (!S_ISREG(buf.st_mode)) {
            LOG_DEBUGF("is not a regular file: %s", path.c_str());
            return ResponseBuilder().status(kStatusForbidden).build();
        }

        std::ifstream ifs(path);
        if (!ifs.is_open()) {
            LOG_DEBUGF("failed to open file: %s", path.c_str());
            return ResponseBuilder().status(kStatusNotFound).build();
        }
        std::stringstream ss;
        ss << ifs.rdbuf();
        return ResponseBuilder().text(ss.str()).build();
    }
} // http
