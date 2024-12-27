#include "response.hpp"
#include "utils/string.hpp"

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

// NOTE: 結構無理やり
std::string http::Response::toString() const {
    std::vector<std::string> lines;

    // status line
    lines.push_back(utils::format("%s %d %s", httpVersion_.c_str(), status_, http::getHttpStatusText(status_).c_str()));

    // field lines
    for (Headers::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        lines.push_back(utils::format("%s: %s", it->first.c_str(), it->second.c_str()));
    }

    // 空行
    lines.push_back("");

    // message body
    lines.push_back(body_);

    const std::string responseMessage = utils::join(lines, "\r\n");
    return responseMessage;
}
