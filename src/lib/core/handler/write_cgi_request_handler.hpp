#ifndef SRC_LIB_CORE_HANDLER_WRITE_CGI_REQUEST_BODY_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_WRITE_CGI_REQUEST_BODY_HANDLER_HPP

#include "event/event_handler.hpp"

class WriteCgiRequestBodyHandler : public IEventHandler {
public:
    explicit WriteCgiRequestBodyHandler(const std::string &cgiBody);
    ~WriteCgiRequestBodyHandler();

    InvokeResult invoke(const Context &ctx);

private:
};

#endif
