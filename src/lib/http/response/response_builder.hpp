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

        Response build();

        ResponseBuilder &status(HttpStatusCode status);
        ResponseBuilder &header(const std::string &name, const std::string &value);

        // Content-Type, Content-Length などの必要な header を自動で付与
        ResponseBuilder &text(const std::string &body, HttpStatusCode status = kStatusOk);
        ResponseBuilder &html(const std::string &body, HttpStatusCode status = kStatusOk);
        ResponseBuilder &redirect(const std::string &location, HttpStatusCode status = kStatusFound);
        ResponseBuilder &file(const std::string &path, HttpStatusCode status = kStatusOk);

    private:
        HttpStatusCode status_;
        std::string httpVersion_; // HTTP version は今のところ固定
        Headers headers_;
        Option<std::string> body_;

        // 通常は header の付与も伴うので、body のみの設定は private にしている
        ResponseBuilder &body(const std::string &body, HttpStatusCode status);
    };
}

#endif
