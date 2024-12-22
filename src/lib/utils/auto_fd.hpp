#ifndef SRC_LIB_AUTO_CLOSE_FD_HPP
#define SRC_LIB_AUTO_CLOSE_FD_HPP

#include "non_copyable.hpp"

/**
 * スマートポインタを真似た fd の wrapper
 * std::auto_ptr とは違い、コピーコンストラクタなどでの move は行わず、コピーを禁止する
 */
class AutoFd : public NonCopyable {
public:
    explicit AutoFd(int fd);
    ~AutoFd();

    // 生の fd を取得する
    int get() const;
    /**
     * std::auto_ptr と同じく、所有権を譲渡する
     * close はしない
     */
    int release();

private:
    int fd_;
};

#endif
