#ifndef SRC_LIB_HTTP_REQUEST_HPP
#define SRC_LIB_HTTP_REQUEST_HPP

#include "method.hpp"
#include "utils/types/option.hpp"
#include <map>

namespace http {
    class Request {
    public:
        typedef std::map<std::string, std::string> Headers;
        explicit Request(HttpMethod method, const std::string &requestTarget,
                         const std::string &httpVersion = "HTTP/1.1",
                         const Headers &headers = std::map<std::string, std::string>(), const std::string &body = "");

        HttpMethod getMethod() const;
        const std::string &getRequestTarget() const;
        const std::string &getHttpVersion() const;
        Option<std::string> getHeader(const std::string &key) const;
        const std::string &getBody() const;

    private:
        HttpMethod method_;
        std::string requestTarget_;
        std::string httpVersion_;
        std::map<std::string, std::string> headers_;
        std::string body_;
    };
}

#endif
