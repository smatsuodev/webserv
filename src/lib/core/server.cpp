#include "server.hpp"
#include "action.hpp"
#include "event/event_notifier.hpp"
#include "./handler/accept_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
#include <map>

Server::Server(const std::string &ip, const unsigned short port) : ip_(ip), port_(port), listener_(ip, port) {}

Server::~Server() {}

void Server::start() {
    state_.getEventNotifier().registerEvent(Event(listener_.getFd(), Event::kRead));
    state_.getEventHandlerRepository().set(listener_.getFd(), new AcceptHandler(listener_));

    LOG_INFOF("server started on port %s:%u", ip_.c_str(), port_);

    while (true) {
        const EventNotifier::WaitEventsResult waitResult = state_.getEventNotifier().waitEvents();
        if (waitResult.isErr()) {
            LOG_ERRORF("EventNotifier::waitEvents failed");
            continue;
        }

        const std::vector<Event> events = waitResult.unwrap();
        for (std::size_t i = 0; i < events.size(); i++) {
            const Event &ev = events[i];
            LOG_DEBUGF("event arrived for fd %d (flags: %x)", ev.getFd(), ev.getTypeFlags());

            Option<IEventHandler *> handler = state_.getEventHandlerRepository().get(ev.getFd());
            if (handler.isNone()) {
                LOG_DEBUGF("event handler for fd %d is not registered", ev.getFd());
                continue;
            }
            Context ctx(state_.getConnectionRepository().get(ev.getFd()), ev);

            const IEventHandler::InvokeResult result = handler.unwrap()->invoke(ctx);
            if (result.isErr()) {
                this->onHandlerError(ctx, result.unwrapErr());
                continue;
            }

            this->executeActions(result.unwrap());
        }
    }
}

void Server::onHandlerError(const Context &ctx, const error::AppError err) {
    // kIOWouldBlock はリトライするので無視
    if (err == error::kIOWouldBlock) {
        return;
    }

    LOG_WARNF("handler error");

    const Event &ev = ctx.getEvent();
    const Option<Connection *> conn = ctx.getConnection();

    // kIOWouldBlock 以外のエラーはコネクションを閉じる
    if (ev.getFd() != listener_.getFd()) {
        state_.getEventNotifier().unregisterEvent(ev);
        state_.getEventHandlerRepository().remove(ev.getFd());
        state_.getConnectionRepository().remove(ev.getFd());
    }
}

void Server::executeActions(std::vector<IAction *> actions) {
    for (std::vector<IAction *>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
        IAction *action = *it;
        action->execute(state_);
        delete action;
    }
}
