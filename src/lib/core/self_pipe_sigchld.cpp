#include "self_pipe_sigchld.hpp"
#include "utils/fd.hpp"
#include <unistd.h>

SelfPipeSigchld *SelfPipeSigchld::instance_ = NULL;

SelfPipeSigchld::SelfPipeSigchld()
    : readFd_(-1), writeFd_(-1), prevSigchldHandler_(SIG_DFL), prevSigpipeHandler_(SIG_DFL) {
    int pipeFds[2];
    if (pipe(pipeFds) == -1) {
        return;
    }

    readFd_ = pipeFds[0];
    writeFd_ = pipeFds[1];

    utils::setNonBlocking(readFd_);
    utils::setNonBlocking(writeFd_);
    utils::setCloseOnExec(readFd_);
    utils::setCloseOnExec(writeFd_);

    // シングルトン的に、このインスタンスをハンドラから参照する
    instance_ = this;

    prevSigchldHandler_ = signal(SIGCHLD, &SelfPipeSigchld::handler);
    prevSigpipeHandler_ = signal(SIGPIPE, SIG_IGN);
}

SelfPipeSigchld::~SelfPipeSigchld() {
    signal(SIGCHLD, prevSigchldHandler_);
    signal(SIGPIPE, prevSigpipeHandler_);
    if (readFd_ != -1) {
        close(readFd_);
    }
    if (writeFd_ != -1) {
        close(writeFd_);
    }
    instance_ = NULL;
}

void SelfPipeSigchld::registerWithEventNotifier(IEventNotifier *notifier) const {
    if (readFd_ < 0) {
        return;
    }
    notifier->registerEvent(Event(readFd_, Event::kRead));
}

// self-pipe を "空" にする (課題の制約上、沢山書き込まれていると空にはならない)
void SelfPipeSigchld::drain() const {
    if (readFd_ < 0) {
        return;
    }
    char buf[256];
    read(readFd_, buf, sizeof(buf));
}

int SelfPipeSigchld::getReadFd() const {
    return readFd_;
}

// signal handler から pipe 経由で通知を送る
void SelfPipeSigchld::handler(int) {
    if (instance_ && instance_->writeFd_ >= 0) {
        const char c = 1;
        write(instance_->writeFd_, &c, 1);
    }
}
