#include "logger.hpp"
#include "utils/logger.hpp"

http::Response http::Logger::intercept(const Request &req, IHandler &next) {
    const std::string method = httpMethodToString(req.getMethod());
    LOG_INFOF("<-- %s %s", method.c_str(), req.getRequestTarget().c_str());
    const Response res = next.serve(req);
    LOG_INFOF("--> %s %s %d", method.c_str(), req.getRequestTarget().c_str(), res.getStatusCode());
    return res;
}
