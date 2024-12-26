#include "request.hpp"

http::Request::Request(const HttpMethod method, const std::string &requestTarget, const std::string &httpVersion,
                       const Headers &headers, const std::string &body)
    : method_(method), requestTarget_(requestTarget), httpVersion_(httpVersion), headers_(headers), body_(body) {}

bool http::Request::operator==(const Request &other) const {
    return method_ == other.method_ && requestTarget_ == other.requestTarget_ && httpVersion_ == other.httpVersion_ &&
        headers_ == other.headers_ && body_ == other.body_;
}

http::HttpMethod http::Request::getMethod() const {
    return method_;
}

const std::string &http::Request::getRequestTarget() const {
    return requestTarget_;
}

const std::string &http::Request::getHttpVersion() const {
    return httpVersion_;
}

Option<std::string> http::Request::getHeader(const std::string &key) const {
    const Headers::const_iterator it = headers_.find(key);
    if (it == headers_.end()) {
        return None;
    }
    return Some(it->second);
}

const std::string &http::Request::getBody() const {
    return body_;
}
