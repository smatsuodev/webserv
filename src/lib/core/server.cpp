#include "server.hpp"
#include "action/action.hpp"
#include "event/event_notifier.hpp"
#include "./handler/accept_handler.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
#include <map>

Server::Server(const config::Config &config) : config_(config) {
    const config::ServerContextList &servers = config_.getServers();
    // キーは (host, port)。本当は値を Address にしたいが、デフォルトコンストラクタがないので無理だった。
    std::map<std::pair<std::string, std::string>, Listener *> listenerMap;
    listeners_.reserve(servers.size()); // 重複があると必要な要素はこれより少ない
    virtualServers_.reserve(servers.size());
    for (config::ServerContextList::const_iterator it = servers.begin(); it != servers.end(); ++it) {
        const std::pair<std::string, std::string> bindPair =
            std::make_pair(it->getHost(), utils::toString(it->getPort()));

        // 同じホスト、ポートの組は listen しない
        Listener *listener = listenerMap[bindPair];
        if (listener == NULL) {
            listener = new Listener(bindPair.first, bindPair.second);
            listenerMap[bindPair] = listener;

            const int fd = listener->getFd();
            listeners_.push_back(listener);
            listenerFds_.insert(fd);

            // Listener の fd に対する read を待ち、AcceptHandler で処理する
            state_.getEventNotifier().registerEvent(Event(fd, Event::kRead));
            state_.getEventHandlerRepository().set(fd, new AcceptHandler(*listener));
        }

        // Virtual Server を作成
        VirtualServer *vs = new VirtualServer(*it, listener->getBindAddress());
        virtualServers_.push_back(vs);
    }
}

Server::~Server() {
    for (std::vector<Listener *>::const_iterator it = listeners_.begin(); it != listeners_.end(); ++it) {
        delete *it;
    }
    for (VirtualServerList::const_iterator it = virtualServers_.begin(); it != virtualServers_.end(); ++it) {
        delete *it;
    }
}

void Server::start() {
    VirtualServerResolverFactory vsResolverFactory(virtualServers_);

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

            Option<Ref<Connection> > conn = state_.getConnectionRepository().get(ev.getFd());
            Option<Ref<IEventHandler> > handler = state_.getEventHandlerRepository().get(ev.getFd());
            if (handler.isNone()) {
                LOG_DEBUGF("event handler for fd %d is not registered", ev.getFd());
                continue;
            }

            const Context ctx(ev, conn, vsResolverFactory);

            const IEventHandler::InvokeResult result = handler.unwrap().get().invoke(ctx);
            if (result.isErr()) {
                this->onHandlerError(ctx, result.unwrapErr());
                continue;
            }

            ActionContext actionCtx(state_);
            Server::executeActions(actionCtx, result.unwrap());
        }
    }
}

/**
 * TODO: 可能ならエラーレスポンスを返す (Internal Server Error, Payload Too Large など)
 * ここでやることではないかもしれない
 */
void Server::onHandlerError(const Context &ctx, const error::AppError err) {
    // IO のエラーは epoll で検知するので、ここでは無視 (EAGAIN の可能性があるため)
    if (err == error::kRecoverable || err == error::kIOUnknown) {
        return;
    }

    // 致命的なエラーはコネクションを切断
    LOG_WARNF("handler error");
    const Event &ev = ctx.getEvent();
    if (listenerFds_.count(ev.getFd()) == 0) {
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
    if (listenerFds_.count(fd) == 0) {
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
