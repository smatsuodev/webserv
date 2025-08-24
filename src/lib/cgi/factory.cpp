#include "factory.hpp"
#include "utils/string.hpp"
#include "utils/types/result.hpp"
#include <sstream>
#include <utils/types/try.hpp>

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

        /**
         * 三項演算子で書くと
         * incompatible operand types ('const types::None' and 'types::Some<string>')
         * というエラーが出たので、if で書いてる
         */
        Option<std::string> body = None;
        if (!req.getBody().empty()) {
            body = Some(req.getBody());
        }
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

        // 独自実装?
        variables.push_back(MetaVariable("DOCUMENT_ROOT", param.documentRoot));

        // Cookie 対応
        const Option<std::string> cookieHeader = req.getHeader("Cookie");
        if (cookieHeader.isSome()) {
            variables.push_back(MetaVariable("HTTP_COOKIE", cookieHeader.unwrap()));
        }

        return cgi::Request::create(variables, body);
    }

    /*
     * document-response = Content-Type [ Status ] *other-field NL response-body
     * Status = "Status:" status-code SP reason-phrase NL
     * status-code = "200" | "302" | "400" | "501" | extension-code
     */
    Result<http::Response, error::AppError> HttpResponseFactory::create(const cgi::Response &res) {
        http::Headers headers(res.getHeaders());

        // 200 がデフォルト、Status ヘッダーがあればそれを使う
        const http::Headers::const_iterator statusHeader = headers.find("Status");
        http::HttpStatusCode status = http::kStatusOk;
        if (statusHeader != headers.end()) {
            status = TRY(HttpResponseFactory::statusFromHeader(statusHeader->second)).unwrapOr(http::kStatusOk);
            // http response header として Status は無効なので、削除する
            headers.erase(statusHeader);
        }

        return Ok(http::Response(status, headers, res.getBody()));
    }

    Result<Option<http::HttpStatusCode>, error::AppError>
    HttpResponseFactory::statusFromHeader(const std::string &headerValue) {
        // "200 OK" の形式。最初の数値を status として取り出す
        std::stringstream ss(headerValue);
        int statusNum;
        ss >> statusNum;
        if (ss.fail()) {
            return Err(error::kUnknown);
        }
        return Ok(http::httpStatusCodeFromInt(statusNum));
    }
}
