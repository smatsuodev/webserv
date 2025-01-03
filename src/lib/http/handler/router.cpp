#include "router.hpp"
#include "matcher.hpp"
#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"

namespace http {
    Router::Router() {}

    Router::~Router() {
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            for (MethodHandlerMap::const_iterator jt = it->second.begin(); jt != it->second.end(); ++jt) {
                delete jt->second;
            }
        }
    }

    void Router::onGet(const std::string &path, IHandler *handler) {
        LOG_DEBUGF("register GET handler for path: %s", path.c_str());
        if (handlers_[path][kMethodGet]) {
            LOG_DEBUGF("handler for GET %s is overwritten", path.c_str());
            delete handlers_[path][kMethodGet];
        }
        handlers_[path][kMethodGet] = handler;
    }

    void Router::onPost(const std::string &path, IHandler *handler) {
        LOG_DEBUGF("register POST handler for path: %s", path.c_str());
        if (handlers_[path][kMethodPost]) {
            LOG_DEBUGF("handler for POST %s is overwritten", path.c_str());
            delete handlers_[path][kMethodPost];
        }
        handlers_[path][kMethodPost] = handler;
    }

    void Router::onDelete(const std::string &path, IHandler *handler) {
        LOG_DEBUGF("register DELETE handler for path: %s", path.c_str());
        if (handlers_[path][kMethodDelete]) {
            LOG_DEBUGF("handler for DELETE %s is overwritten", path.c_str());
            delete handlers_[path][kMethodDelete];
        }
        handlers_[path][kMethodDelete] = handler;
    }

    // NOTE: 現在は GET, POST, DELETE のみ対応
    void Router::on(const std::vector<HttpMethod> &methods, const std::string &path, IHandler *handler) {
        for (std::vector<HttpMethod>::const_iterator it = methods.begin(); it != methods.end(); ++it) {
            switch (*it) {
                case kMethodGet:
                    this->onGet(path, handler);
                    break;
                case kMethodPost:
                    this->onPost(path, handler);
                    break;
                case kMethodDelete:
                    this->onDelete(path, handler);
                    break;
                default:
                    // do nothing
                    break;
            }
        }
    }

    Response Router::serve(const Request &req) {
        // 適切な handler を探す
        // TODO: Matcher を毎回生成し直すのをやめたい
        const Matcher<Path> matcher = this->createMatcher();
        const Option<Path> matchResult = matcher.match(req.getRequestTarget());
        if (matchResult.isNone()) {
            LOG_DEBUGF("no handler found for path: %s", req.getRequestTarget().c_str());
            return ResponseBuilder().status(kStatusNotFound).build();
        }

        // onGet, onPost, onDelete で登録された method のみ許可する
        const MethodHandlerMap &methodHandlers = handlers_[matchResult.unwrap()];
        const MethodHandlerMap::const_iterator it = methodHandlers.find(req.getMethod());
        if (it == methodHandlers.end()) {
            LOG_DEBUGF(
                "method %s is not allowed for path: %s",
                http::httpMethodToString(req.getMethod()).c_str(),
                req.getRequestTarget().c_str()
            );
            return ResponseBuilder().status(kStatusMethodNotAllowed).build();
        }

        // 具体的な処理は登録した handler に委譲
        return it->second->serve(req);
    }

    Matcher<Router::Path> Router::createMatcher() const {
        std::map<Path, Path> paths;
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            paths[it->first] = it->first;
        }
        return Matcher<Path>(paths);
    }
}
