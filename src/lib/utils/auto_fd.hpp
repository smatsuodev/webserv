#ifndef SRC_LIB_AUTO_CLOSE_FD_HPP
#define SRC_LIB_AUTO_CLOSE_FD_HPP

#include "non_copyable.hpp"

/**
 * スマートポインタを真似た fd の wrapper
 * std::auto_ptr とは違い、コピーコンストラクタなどでの move は行わず、コピーを禁止する
 */
class AutoFd : public NonCopyable {
public:
    AutoFd();
    explicit AutoFd(int fd);
    ~AutoFd();

    // 生の fd を取得する
    int get() const;
    // リソースの所有権を放棄する (close はしない)
    int release();
    // リソースを解放し、新たなリソースの所有権を設定する
    void reset(int fd = -1);

    // 暗黙の型変換を利用して、autoFd == -1 のように書けるようにする
    // ReSharper disable once CppNonExplicitConversionOperator
    operator int() const; // NOLINT(*-explicit-constructor)

private:
    int fd_;
};

#endif
