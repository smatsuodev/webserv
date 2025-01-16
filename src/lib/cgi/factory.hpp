#ifndef SRC_LIB_CGI_CONVERTER_HPP
#define SRC_LIB_CGI_CONVERTER_HPP

#include "./request.hpp"
#include "http/request/request.hpp"
#include "transport/address.hpp"

namespace cgi {
    class RequestFactory {
    public:
        // nginx では SERVER_NAME は server_name ディレクティブの先頭 or 空文字
        // subject は PATH_INFO = SCRIPT_NAME を要求しているが、RFC の記述とは異なる
        struct Parameter {
            const http::Request &req;
            const Address &foreignAddress;
            const std::string &serverName;
            const std::string &serverPort;
            const std::string &scriptName;
            const std::string &pathInfo;
        };
        static Result<cgi::Request, error::AppError> create(const Parameter &param);
    };
}

#endif
