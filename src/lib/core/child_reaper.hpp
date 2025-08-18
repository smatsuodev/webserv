#ifndef SRC_LIB_CORE_WEBSERV_CHILD_REAPER_HPP
#define SRC_LIB_CORE_WEBSERV_CHILD_REAPER_HPP

#include "self_pipe_sigchld.hpp"

// 子プロセスの回収
class ChildReaper {
public:
    // 回収したプロセスの pid と status
    struct ReapedProcess {
        pid_t pid;
        int status;
    };

    ChildReaper() {};

    void attachToEventNotifier(IEventNotifier *notifier) const;
    std::vector<ReapedProcess> onSignalEvent() const;
    int getReadFd() const;

private:
    SelfPipeSigchld selfPipe_;
};

#endif
