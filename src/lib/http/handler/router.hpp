#ifndef SRC_LIB_HTTP_HANDLER_ROUTER_HPP
#define SRC_LIB_HTTP_HANDLER_ROUTER_HPP

#include "./handler.hpp"
#include <map>

namespace http {
    class Router : public IHandler {
    public:
        ~Router();

        Result<Response, error::AppError> serve(Request &req);
        // 登録された handler はデストラクタで削除する
        void registerHandler(const std::string &path, IHandler *handler);

    private:
        std::map<std::string, IHandler *> handlers_;

        Option<IHandler *> findHandler(const std::string &path);
    };
}

#endif
