#ifndef SRC_LIB_EVENT_HANDLER_ECHO_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_ECHO_HANDLER_HPP

#include "event/event_handler.hpp"

class EchoHandler : public IEventHandler {
public:
    EchoHandler(const std::string &content);
    InvokeResult invoke(const Context &ctx);

private:
    std::string content_;
};

#endif
