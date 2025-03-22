#include "logger.hpp"
#include "utils/logger.hpp"

http::Response http::Logger::intercept(const Request &req, IHandler &next) {
    const std::string method = httpMethodToString(req.getMethod());
    LOG_INFOF("<-- %s %s", method.c_str(), req.getRequestTarget().c_str());
    const Either<IAction *, Response> serveRes = next.serve(req);
    if (serveRes.isRight()) {
        const Response res = serveRes.unwrapRight();
        LOG_INFOF("--> %s %s %d", method.c_str(), req.getRequestTarget().c_str(), res.getStatusCode());
        return res;
    }

    // TODO: Leftのハンドリング
    throw std::runtime_error("`Logger::intercept()` handling `Left<IAction *>` is not implemented");
}
