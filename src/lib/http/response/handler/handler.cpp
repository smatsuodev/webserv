#include "handler.hpp"

http::IHandler::~IHandler() {}

Result<http::Response, error::AppError> http::Handler::serve(Request &req) {
    // TODO: implement me
    return Err(error::kUnknown);
}
