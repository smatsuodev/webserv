#ifndef SRC_LIB_HTTP_RESPONSE_REQUEST_BUILDER_HPP
#define SRC_LIB_HTTP_RESPONSE_REQUEST_BUILDER_HPP

#include "response.hpp"
#include "http/header.hpp"
#include "http/status.hpp"
#include "utils/types/option.hpp"

namespace http {
    // TODO: テスト書く
    class ResponseBuilder {
    public:
        ResponseBuilder();
        ~ResponseBuilder();

        // HTTP version は今のところ固定

        ResponseBuilder &status(HttpStatusCode status);
        ResponseBuilder &header(const std::string &name, const std::string &value);

        // Content-Type, Content-Length などの必要な header を自動で付与
        ResponseBuilder &text(const std::string &body, HttpStatusCode status = kStatusOk);
        ResponseBuilder &html(const std::string &body, HttpStatusCode status = kStatusOk);
        ResponseBuilder &redirect(const std::string &location, HttpStatusCode status = kStatusFound);

        Response build();

    private:
        HttpStatusCode status_;
        std::string httpVersion_;
        Headers headers_;
        Option<std::string> body_;
    };
}

#endif
