#ifndef SRC_LIB_EVENT_EVENT_NOTIFIER_HPP
#define SRC_LIB_EVENT_EVENT_NOTIFIER_HPP

#include "event.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include <vector>
#include <map>

// TODO: poll, epoll の二重管理が辛いので、今は epoll を無効化
#define USE_EPOLL 0

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
    typedef std::map<int, Event> EventMap;
    EventMap registeredEvents_;

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
