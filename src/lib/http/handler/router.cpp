#include "router.hpp"
#include "matcher.hpp"
#include "http/response/response_builder.hpp"
#include "utils/logger.hpp"

namespace http {
    Router::Router() {}

    Router::~Router() {
        for (HandlerMap::const_iterator it = handlers_.begin(); it != handlers_.end(); ++it) {
            delete it->second;
        }
    }

    void Router::onGet(const std::string &path, IHandler *handler) {
        LOG_DEBUGF("register GET handler for path: %s", path.c_str());
        handlers_[path] = handler;
        allowedMethods_[handler].push_back(kMethodGet);
    }

    void Router::onPost(const std::string &path, IHandler *handler) {
        LOG_DEBUGF("register POST handler for path: %s", path.c_str());
        handlers_[path] = handler;
        allowedMethods_[handler].push_back(kMethodPost);
    }

    void Router::onDelete(const std::string &path, IHandler *handler) {
        LOG_DEBUGF("register DELETE handler for path: %s", path.c_str());
        handlers_[path] = handler;
        allowedMethods_[handler].push_back(kMethodDelete);
    }

    Response Router::serve(const Request &req) {
        // 適切な handler を探す
        // TODO: Matcher を毎回生成し直すのをやめたい
        const Matcher<IHandler *> matcher(handlers_);
        const Option<IHandler *> matchResult = matcher.match(req.getRequestTarget());
        if (matchResult.isNone()) {
            return ResponseBuilder().status(kStatusNotFound).build();
        }
        IHandler *handler = matchResult.unwrap();

        // onGet, onPost, onDelete で登録された method のみ許可する
        const AllowedMethodList &methods = allowedMethods_[handler];
        if (std::find(methods.begin(), methods.end(), req.getMethod()) == methods.end()) {
            return ResponseBuilder().status(kStatusMethodNotAllowed).build();
        }

        // 具体的な処理は登録した handler に委譲
        return handler->serve(req);
    }
}
