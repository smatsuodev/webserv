#include "child_reaper.hpp"
#include <sys/wait.h>
#include <cerrno>

void ChildReaper::attachToEventNotifier(IEventNotifier *notifier) const {
    selfPipe_.registerWithEventNotifier(notifier);
}

std::vector<ChildReaper::ReapedProcess> ChildReaper::onSignalEvent() const {
    std::vector<ReapedProcess> result;

    selfPipe_.drain();
    while (true) {
        int status = 0;
        const pid_t pid = waitpid(-1, &status, WNOHANG);
        ReapedProcess rp = {pid, WEXITSTATUS(status)};
        result.push_back(rp);

        if (pid > 0) {
            continue;
        }
        if (pid == 0) break;
        if (pid == -1 && errno == ECHILD) break;
        if (pid == -1 && errno == EINTR) continue;
        break;
    }
    return result;
}

int ChildReaper::getReadFd() const {
    return selfPipe_.getReadFd();
}
