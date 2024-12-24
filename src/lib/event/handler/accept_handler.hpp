#ifndef SRC_LIB_EVENT_HANDLER_ACCEPT_HANDLER_HPP
#define SRC_LIB_EVENT_HANDLER_ACCEPT_HANDLER_HPP

#include "event_handler.hpp"
#include "event/event_notifier.hpp"
#include "transport/listener.hpp"

class AcceptHandler : public IEventHandler {
public:
    explicit AcceptHandler(EventNotifier &notifier, Listener &listener);

    Result<void, error::AppError> invoke(Context &ctx);

private:
    EventNotifier &notifier_;
    Listener &listener_;
};

#endif
