#ifndef SRC_LIB_EVENT_HANDLER_ACCEPT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_ACCEPT_HANDLER_HPP

#include "event/event_handler.hpp"
#include "event/event_notifier.hpp"
#include "transport/listener.hpp"

class AcceptHandler : public IEventHandler {
public:
    explicit AcceptHandler(Listener &listener);

    InvokeResult invoke(const Context &ctx);

private:
    Listener &listener_;
};

#endif
