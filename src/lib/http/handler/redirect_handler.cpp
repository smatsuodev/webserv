#include "redirect_handler.hpp"
#include "http/response/response_builder.hpp"

namespace http {
    RedirectHandler::RedirectHandler(const std::string &destination) : destination_(destination) {}

    Either<IAction *, Response> RedirectHandler::serve(const RequestContext &ctx) {
        return Right(this->serveInternal(ctx.getRequest()));
    }

    Response RedirectHandler::serveInternal(const Request &) const {
        return ResponseBuilder().status(kStatusFound).header("Location", destination_).build();
    }
}
