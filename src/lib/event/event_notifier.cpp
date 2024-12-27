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

void EventNotifier::registerEvent(const Event &event) {
    const int targetFd = event.getFd();

    epoll_event eev = {};
    eev.events = EventNotifier::toEpollEvents(event) | EPOLLET;
    eev.data.fd = targetFd;
    const int epollOp = registeredFd_.count(targetFd) == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (epoll_ctl(epollFd_, epollOp, targetFd, &eev) == -1) {
        LOG_WARNF("failed to add to epoll fd: %s", std::strerror(errno));
        return;
    }

    registeredFd_.insert(targetFd);
    LOG_DEBUGF("fd %d added to epoll", targetFd);
}

void EventNotifier::unregisterEvent(const Event &event) {
    const int targetFd = event.getFd();

    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, targetFd, NULL) == -1) {
        LOG_WARNF("failed to remove from epoll fd: %s", std::strerror(errno));
        return;
    }

    registeredFd_.erase(targetFd);
    LOG_DEBUGF("fd %d removed from epoll", targetFd);
}

void EventNotifier::triggerPseudoEvent(const Event &event) {
    pseudoEvents_.push_back(event);
}

EventNotifier::WaitEventsResult EventNotifier::waitEvents() {
    epoll_event evs[1024];
    // pseudo-event があれば即座に返ってほしい、そうでなければ待ち続けてほしい
    const int timeout = pseudoEvents_.empty() ? -1 : 0;
    const int numEvents = epoll_wait(epollFd_.get(), evs, 1024, timeout);
    if (numEvents == -1) {
        LOG_ERRORF("epoll_wait failed: %s", std::strerror(errno));
        return Err(error::kUnknown);
    }

    std::vector<Event> events(numEvents);
    for (int i = 0; i < numEvents; i++) {
        events[i] = Event(evs[i].data.fd, EventNotifier::toEventTypeFlags(evs[i].events));
    }

    while (!pseudoEvents_.empty()) {
        // 逆順に追加される
        events.push_back(pseudoEvents_.back());
        pseudoEvents_.pop_back();
    }

    return Ok(events);
}

uint32_t EventNotifier::toEventTypeFlags(const uint32_t epollEvents) {
    uint32_t flags = 0;
    if (epollEvents & EPOLLIN) {
        flags |= Event::kRead;
    }
    if (epollEvents & EPOLLOUT) {
        flags |= Event::kWrite;
    }
    return flags;
}

uint32_t EventNotifier::toEpollEvents(const Event &event) {
    const uint32_t flags = event.getTypeFlags();
    uint32_t events = 0;
    if (flags & Event::kRead) {
        events |= EPOLLIN;
    }
    if (flags & Event::kWrite) {
        events |= EPOLLOUT;
    }
    return events;
}
