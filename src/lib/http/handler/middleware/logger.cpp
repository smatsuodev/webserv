#include "logger.hpp"
#include "utils/logger.hpp"

Either<IAction *, http::Response> http::Logger::intercept(const Request &req, IHandler &next) {
    const std::string method = httpMethodToString(req.getMethod());
    LOG_INFOF("<-- %s %s", method.c_str(), req.getRequestTarget().c_str());

    const Either<IAction *, Response> serveRes = next.serve(req);
    if (serveRes.isLeft()) {
        // Left の場合はまだレスポンスが決定できない
        LOG_INFOF("--> %s %s Pending", method.c_str(), req.getRequestTarget().c_str());
        return serveRes;
    }

    const Response res = serveRes.unwrapRight();
    LOG_INFOF("--> %s %s %d", method.c_str(), req.getRequestTarget().c_str(), res.getStatusCode());
    return Right(res);
}
