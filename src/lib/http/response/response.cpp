#include "response.hpp"

http::Response::Response(const HttpStatusCode status, const std::string &httpVersion, const Headers &headers,
                         const std::string &body)
    : status_(status), httpVersion_(httpVersion), headers_(headers), body_(body) {}

bool http::Response::operator==(const Response &other) const {
    return status_ == other.status_ && httpVersion_ == other.httpVersion_ && headers_ == other.headers_ &&
        body_ == other.body_;
}

http::HttpStatusCode http::Response::getStatusCode() const {
    return status_;
}

const std::string &http::Response::getHttpVersion() const {
    return httpVersion_;
}

const http::Headers &http::Response::getHeaders() const {
    return headers_;
}

const std::string &http::Response::getBody() const {
    return body_;
}
