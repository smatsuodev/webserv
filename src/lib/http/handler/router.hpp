#ifndef SRC_LIB_HTTP_HANDLER_ROUTER_HPP
#define SRC_LIB_HTTP_HANDLER_ROUTER_HPP

#include "handler.hpp"
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

        Response serve(const Request &req);

    private:
        typedef std::map<std::string, IHandler *> HandlerMap;
        typedef std::vector<HttpMethod> AllowedMethodList;
        typedef std::map<IHandler *, AllowedMethodList> AllowedMethodMap;

        HandlerMap handlers_;
        AllowedMethodMap allowedMethods_;
    };
}

#endif
