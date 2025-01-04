#ifndef SRC_LIB_HTTP_HANDLER_ROUTER_HPP
#define SRC_LIB_HTTP_HANDLER_ROUTER_HPP

#include "handler.hpp"
#include "matcher.hpp"
#include "config/config.hpp"

namespace http {
    class Router : public IHandler {
    public:
        Router();
        ~Router();

        // 登録された handler は Router が delete する
        void onGet(const std::string &path, IHandler *handler);
        void onPost(const std::string &path, IHandler *handler);
        void onDelete(const std::string &path, IHandler *handler);
        void on(const std::vector<HttpMethod> &methods, const std::string &path, IHandler *handler);

        Response serve(const Request &req);

    private:
        // (HttpMethod, Path) -> IHandler とするのが自然だが、Method Not Allowed を返すために必要なデータ構造にしている
        typedef std::string Path;
        typedef std::map<HttpMethod, IHandler *> MethodHandlerMap;
        typedef std::map<Path, MethodHandlerMap> HandlerMap;

        HandlerMap handlers_;

        Matcher<Path> createMatcher() const;
    };
}

#endif
