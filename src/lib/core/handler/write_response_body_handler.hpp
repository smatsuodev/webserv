#ifndef SRC_LIB_CORE_HANDLER_WRITE_RESPONSE_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_WRITE_RESPONSE_HANDLER_HPP

#include "event/event_handler.hpp"
#include "http/response/response.hpp"

class WriteResponseHandler : public IEventHandler {
public:
    explicit WriteResponseHandler(const http::Response &response);
    InvokeResult invoke(const Context &ctx);

private:
    std::string responseMessage_;
    size_t totalBytesWritten_;
};

#endif
