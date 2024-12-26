#ifndef SRC_LIB_EVENT_EVENT_NOTIFIER_HPP
#define SRC_LIB_EVENT_EVENT_NOTIFIER_HPP

#include "event.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"

#include <vector>

// epoll の抽象
// とりあえず epoll に特化して実装してある
class EventNotifier {
public:
    EventNotifier();

    // TODO: read/write のどれが対象か選べるようにする
    void registerEvent(const Event &event) const;
    void unregisterEvent(const Event &event) const;

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
};

#endif
