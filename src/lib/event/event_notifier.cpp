#include "event_notifier.hpp"
#include "utils/logger.hpp"
#include <sys/epoll.h>
#include <cstring>
#include <cerrno>

EventNotifier::EventNotifier() : epollFd_(-1) {
    /**
     * epoll_create() creates a new epoll(7) instance.
     * Since Linux 2.6.8, the size argument is ignored, but must be greater than zero.
     */
    epollFd_.reset(epoll_create(1));
    if (epollFd_ == -1) {
        LOG_ERRORF("failed to create epoll fd: %s", std::strerror(errno));
        return;
    }
    LOG_DEBUGF("epoll fd created (fd: %d)", epollFd_.get());
}

void EventNotifier::registerEvent(const Event &event) const {
    const int targetFd = event.getFd();
    epoll_event eev = {};
    eev.events = EPOLLIN | EPOLLET;
    eev.data.fd = targetFd;
    if (epoll_ctl(epollFd_, EPOLL_CTL_ADD, targetFd, &eev) == -1) {
        LOG_ERRORF("failed to add to epoll fd: %s", std::strerror(errno));
        return;
    }

    LOG_DEBUGF("fd %d added to epoll", targetFd);
}

void EventNotifier::unregisterEvent(const Event &event) const {
    const int targetFd = event.getFd();
    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, targetFd, NULL) == -1) {
        LOG_WARNF("failed to remove from epoll fd: %s", std::strerror(errno));
        return;
    }
    LOG_DEBUGF("fd %d removed from epoll", targetFd);
}

EventNotifier::WaitEventsResult EventNotifier::waitEvents() const {
    epoll_event evs[1024];
    const int numEvents = epoll_wait(epollFd_.get(), evs, 1024, -1);
    if (numEvents == -1) {
        LOG_ERRORF("epoll_wait failed: %s", std::strerror(errno));
        return Err(error::kUnknown);
    }

    std::vector<Event> events(numEvents);
    for (int i = 0; i < numEvents; i++) {
        events[i] = Event(evs[i].data.fd);
    }

    return Ok(events);
}
