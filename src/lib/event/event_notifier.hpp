#ifndef SRC_LIB_EVENT_EVENT_NOTIFIER_HPP
#define SRC_LIB_EVENT_EVENT_NOTIFIER_HPP

#include "event.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include <set>
#include <vector>

// epoll の抽象
// とりあえず epoll に特化して実装してある
class EventNotifier {
public:
    EventNotifier();

    void registerEvent(const Event &event);
    void unregisterEvent(const Event &event);

    /**
     * 擬似的にイベントが起きたことにして、waitEvents から返されるようにする
     * 順番は保証しない
     */
    void triggerPseudoEvent(const Event &event);

    typedef Result<std::vector<Event>, error::AppError> WaitEventsResult;
    WaitEventsResult waitEvents();

private:
    AutoFd epollFd_;
    std::vector<Event> pseudoEvents_;
    std::set<int> registeredFd_;

    static uint32_t toEpollEvents(const Event &event);
    static uint32_t toEventTypeFlags(uint32_t epollEvents);
};

#endif
