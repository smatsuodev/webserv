#include "handler.hpp"
#include "utils/logger.hpp"
#include "http/response/response_builder.hpp"
#include <fstream>
#include <sstream>

http::IHandler::~IHandler() {}

Result<http::Response, error::AppError> http::Handler::serve(const http::Request &req) {
    LOG_DEBUGF("request target: %s", req.getRequestTarget().c_str());

    const std::string path = req.getRequestTarget().substr(1);
    if (path.empty()) {
        LOG_DEBUGF("invalid request target: %s", req.getRequestTarget().c_str());
        return Err(error::kUnknown);
    }

    std::ifstream ifs(path);
    if (!ifs.is_open()) {
        LOG_DEBUGF("failed to open file: %s", path.c_str());
        return Err(error::kUnknown);
    }
    std::stringstream ss;
    ss << ifs.rdbuf();
    return Ok(http::ResponseBuilder().text(ss.str()).build());
}
