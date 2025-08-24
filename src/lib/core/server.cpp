#include "server.hpp"
#include "action/action.hpp"
#include "event/event_notifier.hpp"
#include "./handler/accept_handler.hpp"
#include "handler/write_response_body_handler.hpp"
#include "http/response/response_builder.hpp"
#include "transport/listener.hpp"
#include "utils/logger.hpp"
#include "utils/time.hpp"
#include <map>
#include <signal.h>
#include <cstring>

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
            // TODO: read 待ちでええんか?
            state_.getEventHandlerRepository().set(fd, Event::kRead, new AcceptHandler(*listener));
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
            if (errno == EINTR)
                LOG_DEBUG("waitEvents interrupted by signal, retrying...");
            else
                LOG_ERROR("EventNotifier::waitEvents failed");
            continue;
        }

        const std::vector<Event> events = waitResult.unwrap();
        for (std::size_t i = 0; i < events.size(); i++) {
            const Event &ev = events[i];
            LOG_DEBUGF("event arrived for fd %d (flags: %x)", ev.getFd(), ev.getTypeFlags());

            // SIGCHLD を監視する self-pipe のイベント
            // TODO: 関数切り分けて〜
            if (ev.getFd() == state_.getChildReaper().getReadFd()) {
                const std::vector<ChildReaper::ReapedProcess> result = state_.getChildReaper().onSignalEvent();
                if (!result.empty()) LOG_DEBUG("child process reaped");
                for (std::vector<ChildReaper::ReapedProcess>::const_iterator it = result.begin(); it != result.end();
                     ++it) {
                    const Option<CgiProcessRepository::Data> data = state_.getCgiProcessRepository().get(it->pid);
                    state_.getCgiProcessRepository().remove(it->pid);

                    if (it->status == 0) continue;

                    // CGI スクリプトがエラーで終わった場合
                    if (data.isNone()) {
                        LOG_WARNF(
                            "reaped child process %d with non-zero status %d, but no client fd found",
                            it->pid,
                            it->status
                        );
                        continue;
                    }
                    const int clientFd = data.unwrap().clientFd;
                    const int processSocketFd = data.unwrap().processSocketFd;
                    // 子プロセスとのソケットの諸々を解除
                    state_.getEventNotifier().unregisterEvent(Event(processSocketFd, Event::kRead));
                    state_.getEventNotifier().unregisterEvent(Event(processSocketFd, Event::kWrite));
                    state_.getEventHandlerRepository().remove(processSocketFd, Event::kRead);
                    state_.getEventHandlerRepository().remove(processSocketFd, Event::kWrite);
                    state_.getConnectionRepository().remove(processSocketFd);
                    // エラーレスポンスを返す
                    http::ResponseBuilder builder;
                    state_.getEventNotifier().registerEvent(Event(clientFd, Event::kWrite));
                    state_.getEventHandlerRepository().set(
                        clientFd,
                        Event::kWrite,
                        new WriteResponseHandler(builder.status(http::kStatusInternalServerError).build())
                    );
                }
                continue;
            }

            Option<Ref<Connection> > conn = state_.getConnectionRepository().get(ev.getFd());
            const Context ctx(ev, conn, vsResolverFactory);

            invokeHandlers(ctx);
        }

        // タイムアウトチェック
        removeTimeoutHandlers();
    }
}

void Server::invokeHandlers(const Context &ctx) {
    const Event &event = ctx.getEvent();

    const std::vector<Event::EventType> types = {Event::kRead, Event::kWrite};
    // TODO: handler の呼び方が壊れてる
    for (std::vector<Event::EventType>::const_iterator it = types.begin(); it != types.end(); ++it) {
        const Event::EventType type = *it;
        const bool shouldCallHandler = (event.getTypeFlags() & type) != 0;

        if (!(shouldCallHandler || event.isError())) {
            // 待っていないイベントは無視
            continue;
        }

        const Option<Ref<IEventHandler> > handler = state_.getEventHandlerRepository().get(event.getFd(), type);
        if (handler.isNone()) {
            continue;
        }
        this->invokeSingleHandler(ctx, handler.unwrap(), shouldCallHandler);
    }
}

void Server::invokeSingleHandler(const Context &ctx, const Ref<IEventHandler> &handler, const bool shouldCallHandler) {
    const Event &event = ctx.getEvent();

    if (event.isError()) {
        const IEventHandler::ErrorHandleResult result = handler.get().onErrorEvent(ctx, event);
        if (!result.actions.empty()) {
            ActionContext actionCtx(state_);
            executeActions(actionCtx, result.actions);
        }
        if (result.shouldFallback) {
            this->onErrorEvent(event);
            return;
        }
        // この場合はそのまま handler を呼ぶ
    }

    if (!shouldCallHandler) {
        return;
    }

    const Result<std::vector<IAction *>, error::AppError> result = handler.get().invoke(ctx);
    if (result.isErr()) {
        this->onHandlerError(ctx, result.unwrapErr());
        return;
    }
    if (!result.unwrap().empty()) {
        ActionContext actionCtx(state_);
        executeActions(actionCtx, result.unwrap());
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
        state_.getEventHandlerRepository().remove(ev.getFd(), Event::kRead);
        state_.getEventHandlerRepository().remove(ev.getFd(), Event::kWrite);
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
        state_.getEventHandlerRepository().remove(fd, Event::kRead);
        state_.getEventHandlerRepository().remove(fd, Event::kWrite);
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

void Server::removeTimeoutHandlers() {
    removeTimeoutRequestHandlers();
    removeTimeoutCgiProcesses();
}

void Server::removeTimeoutRequestHandlers() {
    const std::time_t currentTime = utils::Time::getCurrentTime();

    const std::vector<int> timedOutFds =
        state_.getConnectionRepository().getTimedOutConnectionFds(currentTime, REQUEST_TIMEOUT_SECONDS, listenerFds_);

    for (std::vector<int>::const_iterator it = timedOutFds.begin(); it != timedOutFds.end(); ++it) {
        const int fd = *it;

        // Read ハンドラーが登録されているかチェック（リクエスト待ち状態）
        const Option<Ref<IEventHandler> > readHandler = state_.getEventHandlerRepository().get(fd, Event::kRead);
        if (readHandler.isNone()) {
            continue;
        }

        LOG_INFOF("Request timeout for fd %d", fd);

        // タイムアウトレスポンスを設定
        http::ResponseBuilder builder;
        http::Response response = builder.status(http::kStatusRequestTimeout).build();

        // Read ハンドラーを削除し、Write ハンドラーを設定
        state_.getEventNotifier().unregisterEvent(Event(fd, Event::kRead));
        state_.getEventHandlerRepository().remove(fd, Event::kRead);
        state_.getEventNotifier().registerEvent(Event(fd, Event::kWrite));
        state_.getEventHandlerRepository().set(fd, Event::kWrite, new WriteResponseHandler(response));
    }
}

void Server::removeTimeoutCgiProcesses() {
    const std::time_t currentTime = utils::Time::getCurrentTime();
    const std::vector<std::pair<pid_t, CgiProcessRepository::Data> > timedOutProcesses =
        state_.getCgiProcessRepository().getTimedOutProcesses(currentTime, CGI_TIMEOUT_SECONDS);

    for (std::vector<std::pair<pid_t, CgiProcessRepository::Data> >::const_iterator it = timedOutProcesses.begin();
         it != timedOutProcesses.end();
         ++it) {
        const pid_t pid = it->first;
        const CgiProcessRepository::Data &data = it->second;

        LOG_INFOF("CGI timeout for pid %d", pid);

        // CGI プロセスを強制終了
        if (kill(pid, SIGTERM) == -1) {
            LOG_WARNF("Failed to terminate CGI process %d: %s", pid, std::strerror(errno));
        }

        // プロセスソケットをクリーンアップ
        const int processSocketFd = data.processSocketFd;
        const int clientFd = data.clientFd;

        state_.getEventNotifier().unregisterEvent(Event(processSocketFd, Event::kRead));
        state_.getEventNotifier().unregisterEvent(Event(processSocketFd, Event::kWrite));
        state_.getEventHandlerRepository().remove(processSocketFd, Event::kRead);
        state_.getEventHandlerRepository().remove(processSocketFd, Event::kWrite);
        state_.getConnectionRepository().remove(processSocketFd);
        state_.getCgiProcessRepository().remove(pid);

        // Gateway Timeout レスポンスを返す
        http::ResponseBuilder builder;
        http::Response response = builder.status(http::kStatusGatewayTimeout).build();
        state_.getEventNotifier().registerEvent(Event(clientFd, Event::kWrite));
        state_.getEventHandlerRepository().set(clientFd, Event::kWrite, new WriteResponseHandler(response));
    }
}
