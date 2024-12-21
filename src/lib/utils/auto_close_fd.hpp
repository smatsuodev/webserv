#ifndef SRC_LIB_AUTO_CLOSE_FD_HPP
#define SRC_LIB_AUTO_CLOSE_FD_HPP

#include "utils/non_copyable.hpp"

/**
 * close 後の使用を避けるため、コピーを禁止する
 * (ただし、raw() を使えばできてしまう)
 */
class AutoCloseFd : public NonCopyable {
public:
    AutoCloseFd(int fd);
    ~AutoCloseFd();

    int raw() const;
    int steal();

private:
    static const int kInvalidFd = -1;
    int fd_;
};

#endif
