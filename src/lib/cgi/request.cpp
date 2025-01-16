#include "request.hpp"
#include "utils/types/result.hpp"

namespace cgi {
    Result<Request, error::AppError>
    Request::create(const std::vector<MetaVariable> &variables, const Option<std::string> &body) {
        // TODO: 必須の変数が含まれているか確認する
        /**
         * 以下は RFC3875 で定義される必須の変数
         * CONTENT_LENGTH, CONTENT_TYPE
         * PATH_INFO, QUERY_STRING, REQUEST_METHOD, SCRIPT_NAME
         * REMOTE_ADDR, GATEWAY_INTERFACE, SERVER_NAME, SERVER_PORT, SERVER_PROTOCOL, SERVER_SOFTWARE
         */
        return Ok(Request(variables, body));
    }

    Request::Request(const std::vector<MetaVariable> &variables, const Option<std::string> &body)
        : variables_(variables), body_(body) {}

    const std::vector<MetaVariable> &Request::getVariables() const {
        return variables_;
    }
    const Option<std::string> &Request::getBody() const {
        return body_;
    }
}
