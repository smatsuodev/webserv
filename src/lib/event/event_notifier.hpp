#ifndef SRC_LIB_EVENT_EVENT_NOTIFIER_HPP
#define SRC_LIB_EVENT_EVENT_NOTIFIER_HPP

#include "event.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include <set>
#include <vector>

class IEventNotifier {
public:
    virtual ~IEventNotifier();

    virtual void registerEvent(const Event &event) = 0;
    virtual void unregisterEvent(const Event &event) = 0;

    /**
     * 擬似的にイベントが起きたことにして、waitEvents から返されるようにする
     * 順番は保証しない
     */
    virtual void triggerPseudoEvent(const Event &event) = 0;

    typedef Result<std::vector<Event>, error::AppError> WaitEventsResult;
    virtual WaitEventsResult waitEvents() = 0;
};

// epoll の抽象
class EpollEventNotifier : public IEventNotifier {
public:
    EpollEventNotifier();

    void registerEvent(const Event &event);
    void unregisterEvent(const Event &event);
    void triggerPseudoEvent(const Event &event);
    WaitEventsResult waitEvents();

private:
    AutoFd epollFd_;
    std::vector<Event> pseudoEvents_;
    std::set<int> registeredFd_;

    static uint32_t toEpollEvents(const Event &event);
    static uint32_t toEventTypeFlags(uint32_t epollEvents);
};

// kqueue の抽象
class KqueueEventNotifier : public IEventNotifier {
public:
    KqueueEventNotifier();

    void registerEvent(const Event &event);
    void unregisterEvent(const Event &event);
    void triggerPseudoEvent(const Event &event);
    WaitEventsResult waitEvents();

private:
    AutoFd kqueueFd_;
    std::vector<Event> pseudoEvents_;
    std::set<int> registeredFd_;

    typedef std::vector<int16_t> KqueueFilters;

    static KqueueFilters toKqueueFilters(const Event &event);
    static uint32_t toEventTypeFlags(const KqueueFilters &filters);
};

#endif
