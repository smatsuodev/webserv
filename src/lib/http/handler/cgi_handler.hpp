#ifndef SRC_LIB_HTTP_HANDLER_CGI_HANDLER_HPP
#define SRC_LIB_HTTP_HANDLER_CGI_HANDLER_HPP

#include "handler.hpp"
#include "cgi/request.hpp"
#include <string>

namespace http {
    // CGI を実行する event handler に繋げるための http handler。これ自体は CGI を実行しない。
    class CgiHandler : public IHandler {
    public:
        // リクエストが CGI スクリプトに対するものでない場合、fallbackHandler を呼び出す
        CgiHandler(const std::string &serverName, IHandler *fallbackHandler)
            : serverName_(serverName), fallbackHandler_(fallbackHandler) {}

        ~CgiHandler() {
            delete fallbackHandler_;
        }

        Either<IAction *, Response> serve(const RequestContext &ctx);

    private:
        std::string serverName_;
        IHandler *fallbackHandler_;

        bool isCgiRequest(const RequestContext &ctx) const;
        Result<cgi::Request, error::AppError> createCgiRequest(const RequestContext &ctx) const;
        std::string getScriptName(const RequestContext &ctx) const;
        std::string getPathInfo(const RequestContext &ctx) const;
    };
}

#endif
