#include "factory.hpp"
#include "utils/string.hpp"
#include "utils/types/result.hpp"

namespace cgi {
    /**
     * 以下が必須:
     * CONTENT_LENGTH, CONTENT_TYPE
     * REQUEST_METHOD, SCRIPT_NAME, PATH_INFO, QUERY_STRING
     * REMOTE_ADDR, GATEWAY_INTERFACE, SERVER_SOFTWARE SERVER_PROTOCOL, SERVER_NAME, SERVER_PORT
     */
    Result<cgi::Request, error::AppError> RequestFactory::create(const Parameter &param) {
        const http::Request &req = param.req;

        std::vector<MetaVariable> variables;
        variables.push_back(MetaVariable("GATEWAY_INTERFACE", "CGI/1.1"));
        variables.push_back(MetaVariable("SERVER_PROTOCOL", req.getHttpVersion()));
        variables.push_back(MetaVariable("SERVER_SOFTWARE", "webserv"));

        // body 関連
        const Option<std::string> body = req.getBody().empty() ? None : Some(req.getBody());
        if (body.isSome()) {
            // Transfer-Encoding: chunked の場合は Content-Length がないので、body の size を直接渡す
            variables.push_back(MetaVariable("CONTENT_LENGTH", utils::toString(body.unwrap().size())));

            const Option<std::string> contentType = req.getHeader("Content-Type");
            if (contentType.isSome()) {
                variables.push_back(MetaVariable("CONTENT_TYPE", contentType.unwrap()));
            }
        }

        variables.push_back(MetaVariable("REQUEST_METHOD", http::httpMethodToString(req.getMethod())));
        variables.push_back(MetaVariable("QUERY_STRING", ""));
        variables.push_back(MetaVariable("SCRIPT_NAME", param.scriptName));
        variables.push_back(MetaVariable("PATH_INFO", param.pathInfo));

        variables.push_back(MetaVariable("REMOTE_ADDR", param.foreignAddress.getIp()));
        variables.push_back(MetaVariable("SERVER_NAME", param.serverName));
        variables.push_back(MetaVariable("SERVER_PORT", param.serverPort));

        return cgi::Request::create(variables, body);
    }
}
