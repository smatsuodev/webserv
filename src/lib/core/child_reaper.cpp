#include "child_reaper.hpp"
#include <sys/wait.h>

void ChildReaper::attachToEventNotifier(IEventNotifier *notifier) const {
    selfPipe_.registerWithEventNotifier(notifier);
}

bool ChildReaper::onSignalEvent() const {
    selfPipe_.drain();
    bool reaped = false;
    while (true) {
        int status = 0;
        const pid_t pid = waitpid(-1, &status, WNOHANG);
        if (pid > 0) {
            reaped = true;
            continue;
        }
        if (pid == 0) break;
        if (pid == -1 && errno == ECHILD) break;
        if (pid == -1 && errno == EINTR) continue;
        break;
    }
    return reaped;
}

int ChildReaper::getReadFd() const {
    return selfPipe_.getReadFd();
}
