#ifndef SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP

#include "event/event_handler.hpp"

class ReadRequestHandler : public IEventHandler {
public:
    InvokeResult invoke(const Context &ctx);
};

#endif
