#include "response.hpp"
#include "utils/types/result.hpp"

namespace cgi {
    Result<Response, error::AppError> Response::create(const Headers &headers, const Option<std::string> &body) {
        if (headers.find("Content-Type") == headers.end()) {
            return Err(error::kValidationInvalidArgs);
        }
        return Ok(Response(headers, body));
    }

    Response::Response(const Headers &headers, const Option<std::string> &body) : headers_(headers), body_(body) {}

    const Headers &Response::getHeaders() const {
        return headers_;
    }
    const Option<std::string> &Response::getBody() const {
        return body_;
    }
}
