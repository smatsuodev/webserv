#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "repository.hpp"
#include "event/event_notifier.hpp"
#include "event/event_handler.hpp"
#include "transport/listener.hpp"

class Server {
public:
    Server(const std::string &ip, unsigned short port);
    ~Server();

    void start();

private:
    std::string ip_;
    unsigned short port_;

    // NOTE: Server の作成と同時に初期化される
    Listener listener_;
    // NOTE: Server の作成と一緒に EventNotifier が初期化される。それでよいのか?
    EventNotifier notifier_;
    ConnectionRepository connRepo_;
    EventHandlerRepository handlerRepo_;

    void onHandlerError(const Context &ctx, error::AppError err);
    void executeActions(std::vector<IAction *> actions);

    /**
     * IAction::execute に this を渡して、メンバを操作させてる
     * できれば friend をやめたい
     */
    friend class AddConnectionAction;
    friend class RemoveConnectionAction;
    friend class RegisterEventHandlerAction;
    friend class UnregisterEventHandlerAction;
    friend class RegisterEventAction;
    friend class UnregisterEventAction;
};

#endif
