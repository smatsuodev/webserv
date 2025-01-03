#include "handler.hpp"
#include "utils/logger.hpp"
#include "http/response/response_builder.hpp"
#include <fstream>
#include <sstream>
#include <cstdio>
#include <sys/stat.h>

http::IHandler::~IHandler() {}

http::Response http::Handler::deleteMethod(const std::string &path) {
    struct stat buf = {0};
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

http::Response http::Handler::serve(const http::Request &req) {
    LOG_DEBUGF("request target: %s", req.getRequestTarget().c_str());
    const std::string path = req.getRequestTarget().substr(1);

    if (path.empty()) {
        LOG_DEBUGF("invalid request target: %s", req.getRequestTarget().c_str());
        return ResponseBuilder().status(kStatusNotFound).build();
    }

    if (req.getMethod() == kMethodDelete) {
        return deleteMethod(path);
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
