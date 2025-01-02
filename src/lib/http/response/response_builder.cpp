#include "response_builder.hpp"
#include "utils/string.hpp"

namespace http {
    ResponseBuilder::ResponseBuilder() : status_(kStatusOk), httpVersion_("HTTP/1.1"), body_(None) {}

    ResponseBuilder::~ResponseBuilder() {}

    ResponseBuilder &ResponseBuilder::status(const HttpStatusCode status) {
        status_ = status;
        return *this;
    }

    ResponseBuilder &ResponseBuilder::header(const std::string &name, const std::string &value) {
        headers_[name] = value;
        return *this;
    }

    ResponseBuilder &ResponseBuilder::text(const std::string &body, const HttpStatusCode status) {
        body_ = Some(body);
        this->status(status);
        this->header("Content-Type", "text/plain; charset=UTF-8");
        this->header("Content-Length", utils::toString(body.size()));
        return *this;
    }

    ResponseBuilder &ResponseBuilder::html(const std::string &body, const HttpStatusCode status) {
        body_ = Some(body);
        this->status(status);
        this->header("Content-Type", "text/html; charset=UTF-8");
        this->header("Content-Length", utils::toString(body.size()));
        return *this;
    }

    ResponseBuilder &ResponseBuilder::redirect(const std::string &location, const HttpStatusCode status) {
        this->status(status);
        this->header("Location", location);
        return *this;
    }

    Response ResponseBuilder::build() {
        if (body_.isNone()) {
            this->text("");
        }
        return Response(status_, httpVersion_, headers_, body_.unwrap());
    }
}
