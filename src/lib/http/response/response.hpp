#ifndef SRC_LIB_HTTP_RESPONSE_RESPONSE_HPP
#define SRC_LIB_HTTP_RESPONSE_RESPONSE_HPP

#include "../status.hpp"
#include "http/header.hpp"

namespace http {
    class Response {
    public:
        explicit Response(HttpStatusCode status, const std::string &httpVersion = "HTTP/1.1",
                          const Headers &headers = Headers(), const std::string &body = "");
        bool operator==(const Response &other) const;

        HttpStatusCode getStatusCode() const;
        const std::string &getHttpVersion() const;
        const Headers &getHeaders() const;
        const std::string &getBody() const;

    private:
        HttpStatusCode status_;
        std::string httpVersion_;
        Headers headers_;
        std::string body_;
    };
}


#endif
