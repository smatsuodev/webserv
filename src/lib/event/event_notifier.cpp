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
    eev.events = EpollEventNotifier::toEpollEvents(event) | EPOLLET;
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

void EpollEventNotifier::triggerPseudoEvent(const Event &event) {
    pseudoEvents_.push_back(event);
}

EpollEventNotifier::WaitEventsResult EpollEventNotifier::waitEvents() {
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
        events[i] = Event(evs[i].data.fd, EpollEventNotifier::toEventTypeFlags(evs[i].events));
    }

    while (!pseudoEvents_.empty()) {
        // 逆順に追加される
        events.push_back(pseudoEvents_.back());
        pseudoEvents_.pop_back();
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
    return events;
}

#elif defined(__APPLE__)

#include <sys/event.h>

KqueueEventNotifier::KqueueEventNotifier() : kqueueFd_(-1) {
    LOG_INFO("using event method: kqueue");

    kqueueFd_.reset(kqueue());
    if (kqueueFd_ == -1) {
        LOG_ERRORF("failed to create kqueue fd: %s", std::strerror(errno));
        return;
    }
    LOG_DEBUGF("kqueue fd created (fd: %d)", kqueueFd_.get());
}

void KqueueEventNotifier::registerEvent(const Event &event) {
    const int targetFd = event.getFd();

    const KqueueFilters filters = KqueueEventNotifier::toKqueueFilters(event);
    std::vector<struct kevent> changes(filters.size());
    for (std::size_t i = 0; i < filters.size(); i++) {
        EV_SET(&changes[i], targetFd, filters[i], EV_ADD | EV_ENABLE, 0, 0, NULL);
    }

    if (kevent(kqueueFd_, changes.data(), static_cast<int>(changes.size()), NULL, 0, NULL) == -1) {
        LOG_ERRORF("failed to register event: %s", std::strerror(errno));
        return;
    }

    registeredFd_.insert(targetFd);
    LOG_DEBUGF("fd %d added to kqueue", targetFd);
}

void KqueueEventNotifier::unregisterEvent(const Event &event) {
    const int targetFd = event.getFd();

    const KqueueFilters filters = KqueueEventNotifier::toKqueueFilters(event);
    std::vector<struct kevent> changes(filters.size());
    for (std::size_t i = 0; i < filters.size(); i++) {
        EV_SET(&changes[i], targetFd, filters[i], EV_DELETE, 0, 0, NULL);
    }

    if (kevent(kqueueFd_, changes.data(), static_cast<int>(changes.size()), NULL, 0, NULL) == -1) {
        LOG_WARNF("failed to remove from kqueue: %s", std::strerror(errno));
        return;
    }

    registeredFd_.erase(targetFd);
    LOG_DEBUGF("fd %d removed from kqueue", targetFd);
}

void KqueueEventNotifier::triggerPseudoEvent(const Event &event) {
    pseudoEvents_.push_back(event);
}

KqueueEventNotifier::WaitEventsResult KqueueEventNotifier::waitEvents() {
    struct kevent kevList[1024];
    const timespec timeout = {};

    const int numEvents = kevent(kqueueFd_, NULL, 0, kevList, 1024, pseudoEvents_.empty() ? NULL : &timeout);
    if (numEvents == -1) {
        LOG_ERRORF("kevent failed: %s", std::strerror(errno));
        return Err(error::kUnknown);
    }

    // epoll と違って、filter 毎に別のイベントとして返るので、fd 毎にまとめる
    std::map<int, KqueueFilters> filtersMap;
    for (int i = 0; i < numEvents; i++) {
        filtersMap[static_cast<int>(kevList[i].ident)].push_back(kevList[i].filter);
    }

    std::vector<Event> events;
    for (std::map<int, KqueueFilters>::const_iterator it = filtersMap.begin(); it != filtersMap.end(); ++it) {
        const uint32_t flags = KqueueEventNotifier::toEventTypeFlags(it->second);
        events.push_back(Event(it->first, flags));
    }

    while (!pseudoEvents_.empty()) {
        events.push_back(pseudoEvents_.back());
        pseudoEvents_.pop_back();
    }

    return Ok(events);
}

KqueueEventNotifier::KqueueFilters KqueueEventNotifier::toKqueueFilters(const Event &event) {
    std::vector<int16_t> filters;
    const uint32_t typeFlags = event.getTypeFlags();
    if (typeFlags & Event::kRead) {
        filters.push_back(EVFILT_READ);
    }
    if (typeFlags & Event::kWrite) {
        filters.push_back(EVFILT_WRITE);
    }
    return filters;
}

uint32_t KqueueEventNotifier::toEventTypeFlags(const KqueueFilters &filters) {
    uint32_t flags = 0;
    for (KqueueFilters::const_iterator it = filters.begin(); it != filters.end(); ++it) {
        switch (*it) {
            case EVFILT_READ:
                flags |= Event::kRead;
                break;
            case EVFILT_WRITE:
                flags |= Event::kWrite;
                break;
            default:
                // do nothing
                break;
        }
    }
    return flags;
}

#endif
