#include "server.hpp"
#include "action.hpp"
#include "event/event_notifier.hpp"
#include "./handler/accept_handler.hpp"
#include "http/handler/redirect_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"

// TODO: config を元に listener のインスタンスを作る
Server::Server(const config::Config &config)
    : config_(config), resolver_(config), listener_(Address("0.0.0.0", 8080)) {}

Server::~Server() {}

void Server::start() {
    // TODO: 複数 virtual server は未対応
    const config::ServerContext &serverConfig = config_.getServers()[0];

    // router を構築
    http::Router router = Server::createRouter(serverConfig);

    // 最初の event, event handler
    state_.getEventNotifier().registerEvent(Event(listener_.getFd(), Event::kRead));
    state_.getEventHandlerRepository().set(listener_.getFd(), new AcceptHandler(listener_));

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

            if (ev.isError()) {
                this->onErrorEvent(ev);
                continue;
            }

            Option<Ref<IEventHandler> > handler = state_.getEventHandlerRepository().get(ev.getFd());
            if (handler.isNone()) {
                LOG_DEBUGF("event handler for fd %d is not registered", ev.getFd());
                continue;
            }
            Context ctx(state_.getConnectionRepository().get(ev.getFd()), ev, resolver_);

            const IEventHandler::InvokeResult result = handler.unwrap().get().invoke(ctx);
            if (result.isErr()) {
                this->onHandlerError(ctx, result.unwrapErr());
                continue;
            }

            // TODO: 本来は特定の server の router を探して渡す
            ActionContext actionCtx(state_, router);
            Server::executeActions(actionCtx, result.unwrap());
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
    if (!event.isError()) {
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

void Server::executeActions(ActionContext &actionCtx, std::vector<IAction *> actions) {
    for (std::vector<IAction *>::const_iterator it = actions.begin(); it != actions.end(); ++it) {
        IAction *action = *it;
        action->execute(actionCtx);
        delete action;
    }
}

http::Router Server::createRouter(const config::ServerContext &serverConfig) {
    http::Router router;

    config::LocationContextList locations = serverConfig.getLocations();
    for (config::LocationContextList::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const config::LocationContext &location = *it;
        http::IHandler *handler = NULL;
        if (location.getRedirect().isSome()) {
            handler = new http::RedirectHandler(location.getRedirect().unwrap());
        } else {
            // TODO: 設定を渡す
            handler = new http::Handler();
        }
        router.on(location.getAllowedMethods(), location.getPath(), handler);
    }

    return router;
}
