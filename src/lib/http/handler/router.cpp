#include "router.hpp"
#include "matcher.hpp"
#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"
#include <set>

namespace http {
    Router::Router() {
        handlersChain_.push_back(new InternalRouter(handlers_));
    }

    // TODO: リソース管理が煩雑なのでリファクタしたい
    Router::~Router() {
        // メソッドに同じ handler が登録されていると double free になることに注意
        std::set<IHandler *> deleted;
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            for (MethodHandlerMap::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
                if (deleted.count(jt->second) == 0) {
                    delete jt->second;
                    deleted.insert(jt->second);
                }
            }
        }

        for (MiddlewareList::const_iterator it = middlewares_.begin(); it != middlewares_.end(); ++it) {
            delete *it;
        }

        for (std::vector<IHandler *>::iterator it = handlersChain_.begin(); it != handlersChain_.end(); ++it) {
            delete *it;
        }
    }

    void Router::on(const HttpMethod method, const std::string &path, IHandler *handler) {
        const std::string methodStr = httpMethodToString(method);
        LOG_DEBUGF("register %s handler for path: %s", methodStr.c_str(), path.c_str());
        if (handlers_[path][method]) {
            LOG_DEBUGF("handler for %s %s is overwritten", methodStr.c_str(), path.c_str());
            delete handlers_[path][method];
        }
        handlers_[path][method] = handler;
    }

    void Router::on(const std::vector<HttpMethod> &methods, const std::string &path, IHandler *handler) {
        for (std::vector<HttpMethod>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
            this->on(*it, path, handler);
        }
    }

    void Router::use(IMiddleware *middleware) {
        middlewares_.push_back(middleware);
        handlersChain_.push_back(new ChainHandler(*middleware, *handlersChain_.back()));
    }

    Either<IAction *, Response> Router::serve(const Request &req) {
        return handlersChain_.back()->serve(req);
    }

    /* internal */

    Router::InternalRouter::InternalRouter(HandlerMap &handlers) : handlers_(handlers) {}

    Either<IAction *, Response> Router::InternalRouter::serve(const Request &req) {
        // 適切な handler を探す
        // TODO: Matcher を毎回生成し直すのをやめたい
        const Matcher<Path> matcher = this->createMatcher();
        const Option<Path> matchResult = matcher.match(req.getRequestTarget());
        if (matchResult.isNone()) {
            LOG_DEBUGF("no handler found for path: %s", req.getRequestTarget().c_str());
            return Right(ResponseBuilder().status(kStatusNotFound).build());
        }

        // on で登録された method のみ許可する
        const MethodHandlerMap &methodHandlers = handlers_[matchResult.unwrap()];
        const MethodHandlerMap::const_iterator it = methodHandlers.find(req.getMethod());
        if (it == methodHandlers.end()) {
            LOG_DEBUGF(
                "method %s is not allowed for path: %s",
                http::httpMethodToString(req.getMethod()).c_str(),
                req.getRequestTarget().c_str()
            );
            return Right(ResponseBuilder().status(kStatusMethodNotAllowed).build());
        }

        // 具体的な処理は登録した handler に委譲
        return it->second->serve(req);
    }

    Matcher<Router::Path> Router::InternalRouter::createMatcher() const {
        std::map<Path, Path> paths;
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            paths[it->first] = it->first;
        }
        return Matcher<Path>(paths);
    }

    Router::ChainHandler::ChainHandler(IMiddleware &middleware, IHandler &next)
        : middleware_(middleware), next_(next) {}

    Either<IAction *, Response> Router::ChainHandler::serve(const Request &req) {
        return middleware_.intercept(req, next_);
    }
}
