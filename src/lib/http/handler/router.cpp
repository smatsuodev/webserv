#include "router.hpp"
#include "utils/logger.hpp"

http::Router::~Router() {
    for (std::map<std::string, IHandler *>::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
        delete it->second;
    }
}

Result<http::Response, error::AppError> http::Router::serve(Request &req) {
    LOG_DEBUGF("http::Router::serve: %s", req.getRequestTarget().c_str()); // method も出したい

    const Option<IHandler *> handler = this->findHandler(req.getRequestTarget());
    if (handler.isNone()) {
        LOG_DEBUGF("http::Router::serve: handler not found");
        // レスポンスはこれで正しいのか?
        return Ok(Response(kStatusNotFound));
    }
    return handler.unwrap()->serve(req);
}

void http::Router::registerHandler(const std::string &path, IHandler *handler) {
    handlers_.insert(std::make_pair(path, handler));
}

// TODO: 最長一致にする
Option<http::IHandler *> http::Router::findHandler(const std::string &path) {
    const std::map<std::string, IHandler *>::const_iterator it = handlers_.find(path);
    if (it == handlers_.end()) {
        return None;
    }
    return Some(it->second);
}
