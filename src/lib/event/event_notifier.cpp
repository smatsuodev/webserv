#include "event_notifier.hpp"
#include "utils/logger.hpp"
#include <cstring>
#include <cerrno>
#include <map>
#include <string>

IEventNotifier::~IEventNotifier() {}

#if defined(__linux__)

#include <sys/epoll.h>

EpollEventNotifier::EpollEventNotifier() : epollFd_(-1) {
    LOG_INFO("using event method: epoll");

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

void EpollEventNotifier::registerEvent(const Event &event) {
    const int targetFd = event.getFd();

    epoll_event eev = {};
    eev.events = EpollEventNotifier::toEpollEvents(event) | EPOLLERR | EPOLLET;
    eev.data.fd = targetFd;
    const int epollOp = registeredFd_.count(targetFd) == 0 ? EPOLL_CTL_ADD : EPOLL_CTL_MOD;
    if (epoll_ctl(epollFd_, epollOp, targetFd, &eev) == -1) {
        LOG_WARNF("failed to add to epoll fd: %s", std::strerror(errno));
        return;
    }

    registeredFd_.insert(targetFd);
    LOG_DEBUGF("fd %d added to epoll", targetFd);
}

void EpollEventNotifier::unregisterEvent(const Event &event) {
    const int targetFd = event.getFd();

    if (epoll_ctl(epollFd_, EPOLL_CTL_DEL, targetFd, NULL) == -1) {
        LOG_WARNF("failed to remove from epoll fd: %s", std::strerror(errno));
        return;
    }

    registeredFd_.erase(targetFd);
    LOG_DEBUGF("fd %d removed from epoll", targetFd);
}

EpollEventNotifier::WaitEventsResult EpollEventNotifier::waitEvents() {
    epoll_event evs[1024];
    const int numEvents = epoll_wait(epollFd_.get(), evs, 1024, -1);
    if (numEvents == -1) {
        LOG_ERRORF("epoll_wait failed: %s", std::strerror(errno));
        return Err(error::kUnknown);
    }

    std::vector<Event> events(numEvents);
    for (int i = 0; i < numEvents; i++) {
        events[i] = Event(evs[i].data.fd, EpollEventNotifier::toEventTypeFlags(evs[i].events));
    }

    return Ok(events);
}

uint32_t EpollEventNotifier::toEventTypeFlags(const uint32_t epollEvents) {
    uint32_t flags = 0;
    if (epollEvents & EPOLLIN) {
        flags |= Event::kRead;
    }
    if (epollEvents & EPOLLOUT) {
        flags |= Event::kWrite;
    }
    if (epollEvents & EPOLLERR) {
        flags |= Event::kError;
    }
    return flags;
}

uint32_t EpollEventNotifier::toEpollEvents(const Event &event) {
    const uint32_t flags = event.getTypeFlags();
    uint32_t events = 0;
    if (flags & Event::kRead) {
        events |= EPOLLIN;
    }
    if (flags & Event::kWrite) {
        events |= EPOLLOUT;
    }
    if (flags & Event::kError) {
        events |= EPOLLERR;
    }
    return events;
}

#else

#include <poll.h>

PollEventNotifier::PollEventNotifier() {
    LOG_INFO("using event method: poll");
}

void PollEventNotifier::registerEvent(const Event &event) {
    registeredEvents_[event.getFd()] = event;
    LOG_DEBUGF("fd %d added to poll", event.getFd());
}

void PollEventNotifier::unregisterEvent(const Event &event) {
    registeredEvents_.erase(event.getFd());
    LOG_DEBUGF("fd %d removed from poll", event.getFd());
}

IEventNotifier::WaitEventsResult PollEventNotifier::waitEvents() {
    std::vector<pollfd> fds;
    for (EventMap::const_iterator it = registeredEvents_.begin(); it != registeredEvents_.end(); ++it) {
        pollfd pfd = {};
        pfd.fd = it->first;
        // TODO: POLLIN | POLLOUT を通知させる
        pfd.events = static_cast<short>(PollEventNotifier::toPollEvents(it->second) | POLLERR);
        fds.push_back(pfd);
    }

    const int result = poll(fds.data(), fds.size(), -1);
    if (result == -1) {
        LOG_ERRORF("poll failed: %s", std::strerror(errno));
        return Err(error::kUnknown);
    }

    std::vector<Event> events;
    for (std::vector<pollfd>::const_iterator it = fds.begin(); it < fds.end(); ++it) {
        const pollfd pfd = *it;
        if (pfd.revents == 0) {
            continue;
        }
        const uint32_t flags = PollEventNotifier::toEventTypeFlags(pfd.revents);
        if (flags == 0) {
            continue;
        }

        // TODO: in/out 両方通知するようになったら、期待する event だけ filter する
        const Event ev(pfd.fd, flags);
        events.push_back(ev);
    }

    return Ok(events);
}

short PollEventNotifier::toPollEvents(const Event &event) {
    const uint32_t flags = event.getTypeFlags();
    short events = 0;
    if (flags & Event::kRead) {
        events |= POLLIN;
    }
    if (flags & Event::kWrite) {
        events |= POLLOUT;
    }
    if (flags & Event::kError) {
        events |= POLLERR;
    }
    return events;
}

uint32_t PollEventNotifier::toEventTypeFlags(const short pollEvents) {
    uint32_t flags = 0;
    if (pollEvents & POLLIN) {
        flags |= Event::kRead;
    }
    if (pollEvents & POLLOUT) {
        flags |= Event::kWrite;
    }
    if (pollEvents & POLLERR) {
        flags |= Event::kError;
    }
    return flags;
}

#endif
