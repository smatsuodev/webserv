#include "server.hpp"
#include "action.hpp"
#include "event/event_notifier.hpp"
#include "./handler/accept_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"

Server::Server(const std::string &ip, const unsigned short port) : ip_(ip), port_(port), listener_(ip, port) {}

Server::~Server() {}

void Server::start() {
    state_.getEventNotifier().registerEvent(Event(listener_.getFd(), Event::kRead));
    state_.getEventHandlerRepository().set(listener_.getFd(), new AcceptHandler(listener_));

    LOG_INFOF("server started on port %s:%u", ip_.c_str(), port_);

    while (true) {
        const IEventNotifier::WaitEventsResult waitResult = state_.getEventNotifier().waitEvents();
        if (waitResult.isErr()) {
            LOG_ERRORF("EventNotifier::waitEvents failed");
            continue;
        }

        const std::vector<Event> events = waitResult.unwrap();
        for (std::size_t i = 0; i < events.size(); i++) {
            const Event &ev = events[i];
            LOG_DEBUGF("event arrived for fd %d (flags: %x)", ev.getFd(), ev.getTypeFlags());

            if (ev.getTypeFlags() & Event::kError) {
                this->onErrorEvent(ev);
                continue;
            }

            Option<Ref<IEventHandler> > handler = state_.getEventHandlerRepository().get(ev.getFd());
            if (handler.isNone()) {
                LOG_DEBUGF("event handler for fd %d is not registered", ev.getFd());
                continue;
            }
            Context ctx(state_.getConnectionRepository().get(ev.getFd()), ev);

            const IEventHandler::InvokeResult result = handler.unwrap().get().invoke(ctx);
            if (result.isErr()) {
                this->onHandlerError(ctx, result.unwrapErr());
                continue;
            }

            this->executeActions(result.unwrap());
        }
    }
}

void Server::onHandlerError(const Context &ctx, const error::AppError err) {
    // IO のエラーは epoll で検知するので、ここでは無視 (EAGAIN の可能性があるため)
    if (err == error::kRecoverable || err == error::kIOUnknown) {
        return;
    }

    LOG_WARNF("handler error");

    const Event &ev = ctx.getEvent();

    // kIOWouldBlock 以外のエラーはコネクションを閉じる
    if (ev.getFd() != listener_.getFd()) {
        state_.getEventNotifier().unregisterEvent(ev);
        state_.getEventHandlerRepository().remove(ev.getFd());
        state_.getConnectionRepository().remove(ev.getFd());
    }
}

void Server::onErrorEvent(const Event &event) {
    if (!(event.getTypeFlags() & Event::kError)) {
        // kError 以外は無視
        return;
    }

    const int fd = event.getFd();
    LOG_WARNF("error event arrived for fd %d", fd);

    // (たぶん) 継続不可なので cleanup
    if (event.getFd() != listener_.getFd()) {
        state_.getEventNotifier().unregisterEvent(event);
        state_.getEventHandlerRepository().remove(fd);
        state_.getConnectionRepository().remove(fd);
    }
}

void Server::executeActions(std::vector<IAction *> actions) {
    for (std::vector<IAction *>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
        // TODO: 毎回 router を作り直すのをやめる
        http::Router router;
        router.onGet("/", new http::Handler());
        router.onDelete("/", new http::Handler());

        IAction *action = *it;
        ActionContext ctx(state_, router);
        action->execute(ctx);
        delete action;
    }
}
