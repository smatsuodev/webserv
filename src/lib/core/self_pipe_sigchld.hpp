#ifndef SRC_LIB_CORE_WEBSERV_SELF_PIPE_SIGCHLD_HPP
#define SRC_LIB_CORE_WEBSERV_SELF_PIPE_SIGCHLD_HPP

#include "event/event_notifier.hpp"
#include <sys/signal.h>

class SelfPipeSigchld {
public:
    SelfPipeSigchld();
    ~SelfPipeSigchld();

    void registerWithEventNotifier(IEventNotifier *notifier) const;
    void drain() const;
    int getReadFd() const;

private:
    int readFd_;
    int writeFd_;
    sig_t prevSigchldHandler_;
    sig_t prevSigpipeHandler_;
    static SelfPipeSigchld *instance_;

    static void handler(int signum);
};

#endif
