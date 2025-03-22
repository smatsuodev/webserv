#include "redirect_handler.hpp"
#include "http/response/response_builder.hpp"

namespace http {
    RedirectHandler::RedirectHandler(const std::string &destination) : destination_(destination) {}

    Either<IAction *, Response> RedirectHandler::serve(const Request &) {
        return Right(ResponseBuilder().status(kStatusFound).header("Location", destination_).build());
    }
}
