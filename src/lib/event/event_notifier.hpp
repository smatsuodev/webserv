#ifndef SRC_LIB_EVENT_EVENT_NOTIFIER_HPP
#define SRC_LIB_EVENT_EVENT_NOTIFIER_HPP

#include "event.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include <set>
#include <vector>
#include <map>

/**
 * TODO: subject では read/write 両方を監視するよう指定されている
 * インターフェースは変えずに、waitEvents の結果を filter することで対応したい
 */

class IEventNotifier {
public:
    virtual ~IEventNotifier();

    virtual void registerEvent(const Event &event) = 0;
    virtual void unregisterEvent(const Event &event) = 0;

    typedef Result<std::vector<Event>, error::AppError> WaitEventsResult;
    virtual WaitEventsResult waitEvents() = 0;
};

// epoll の抽象
class EpollEventNotifier : public IEventNotifier {
public:
    EpollEventNotifier();

    void registerEvent(const Event &event);
    void unregisterEvent(const Event &event);
    WaitEventsResult waitEvents();

private:
    AutoFd epollFd_;
    std::set<int> registeredFd_;

    static uint32_t toEpollEvents(const Event &event);
    static uint32_t toEventTypeFlags(uint32_t epollEvents);
};

class PollEventNotifier : public IEventNotifier {
public:
    PollEventNotifier();

    void registerEvent(const Event &event);
    void unregisterEvent(const Event &event);
    WaitEventsResult waitEvents();

private:
    typedef std::map<int, Event> EventMap;
    EventMap registeredEvents_;

    static short toPollEvents(const Event &event);
    static uint32_t toEventTypeFlags(short pollEvents);
};

#endif
