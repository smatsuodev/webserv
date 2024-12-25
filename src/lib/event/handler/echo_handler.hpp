#ifndef SRC_LIB_EVENT_HANDLER_ECHO_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_ECHO_HANDLER_HPP
#include "event_handler.hpp"

class EchoHandler : public IEventHandler {
public:
    EchoHandler();

    Result<void, error::AppError> invoke(Context &ctx);

private:
};

#endif
