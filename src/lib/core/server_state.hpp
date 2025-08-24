#ifndef SRC_LIB_CORE_SERVER_STATE_HPP
#define SRC_LIB_CORE_SERVER_STATE_HPP

#include "child_reaper.hpp"
#include "event/event_notifier.hpp"
#include "event/event_handler.hpp"
#include "transport/connection.hpp"
#include "utils/types/option.hpp"
#include <map>
#include <set>
#include <vector>
#include <ctime>

// TODO: 共通化するべき?

class ConnectionRepository : public NonCopyable {
public:
    ConnectionRepository();
    ~ConnectionRepository();

    Option<Ref<Connection> > get(int fd);
    /**
     * Connection#getFd があるので、fd はなくてもいい
     * インターフェースの統一のためにある
     */
    void set(int fd, Connection *conn);
    void remove(int fd);

    // タイムアウトしたコネクションのFDを取得
    std::vector<int>
    getTimedOutConnectionFds(std::time_t currentTime, double timeoutSeconds, const std::set<int> &excludeFds) const;

private:
    std::map<int, Connection *> connections_;
};

class EventHandlerRepository : public NonCopyable {
public:
    EventHandlerRepository();
    ~EventHandlerRepository();

    // get, set, remove が受け取る Event::EventType は、どれか 1 bit が立ったフラグのみを想定している (和はダメ)
    Option<Ref<IEventHandler> > get(int fd, Event::EventType type);
    void set(int fd, Event::EventType type, IEventHandler *handler);
    void remove(int fd, Event::EventType type);

private:
    typedef std::pair<int, Event::EventType> Key;
    std::map<Key, IEventHandler *> handlers_;
};

// 名前が微妙
class CgiProcessRepository : public NonCopyable {
public:
    struct Data {
        int clientFd;
        int processSocketFd;
        std::time_t startTime;
    };

    CgiProcessRepository() {}

    Option<Data> get(pid_t pid);
    void set(pid_t pid, Data data);
    void remove(pid_t pid);

    // タイムアウトしたプロセスを取得
    std::vector<std::pair<pid_t, Data> > getTimedOutProcesses(std::time_t currentTime, double timeoutSeconds) const;

private:
    std::map<pid_t, Data> pidToData_;
};

class ServerState {
public:
    ServerState();

    IEventNotifier &getEventNotifier();
    ConnectionRepository &getConnectionRepository();
    EventHandlerRepository &getEventHandlerRepository();
    CgiProcessRepository &getCgiProcessRepository();
    ChildReaper &getChildReaper();

private:
    // EventNotifier はあんまり state っぽくない
#if defined(__linux__) && USE_EPOLL
    EpollEventNotifier notifier_;
#else
    PollEventNotifier notifier_;
#endif

    ChildReaper reaper_;

    ConnectionRepository connRepo_;
    EventHandlerRepository handlerRepo_;
    CgiProcessRepository cgiProcessRepo_;
};

#endif
