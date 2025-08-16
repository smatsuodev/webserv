#ifndef SRC_LIB_CORE_WEBSERV_CHILD_REAPER_HPP
#define SRC_LIB_CORE_WEBSERV_CHILD_REAPER_HPP

#include "self_pipe_sigchld.hpp"

// 子プロセスの回収
class ChildReaper {
public:
    ChildReaper() {};

    void attachToEventNotifier(IEventNotifier *notifier) const;
    bool onSignalEvent() const;
    int getReadFd() const;

private:
    SelfPipeSigchld selfPipe_;
};

#endif
