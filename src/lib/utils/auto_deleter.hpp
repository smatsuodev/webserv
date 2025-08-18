#ifndef SRC_LIB_UTILS_AUTO_DELETER_HPP
#define SRC_LIB_UTILS_AUTO_DELETER_HPP

#include "non_copyable.hpp"
#include <cstddef>

// deleter 付きの std::auto_ptr のようなクラス
// NOTE: 使用箇所が限定的なので、あんまりちゃんと作り込んでない
template <typename T>
class AutoDeleter : public NonCopyable {
public:
    typedef void (*Deleter)(T *);

    explicit AutoDeleter(T *ptr = NULL, const Deleter deleter = NULL) : ptr_(ptr), deleter_(deleter) {}

    virtual ~AutoDeleter() {
        this->reset();
    }

    // リソースへのアクセス
    T *get() const {
        return ptr_;
    }

    // 所有権を放棄
    T *release() {
        T *res = ptr_;
        ptr_ = NULL;
        return res;
    }

    // 新しいリソースの設定
    void reset(T *newPtr = NULL) {
        if (ptr_ != NULL && deleter_) {
            deleter_(ptr_);
        }
        ptr_ = newPtr;
    }

    // 暗黙の型変換で、元の型と同じように扱えるようにする
    // ReSharper disable once CppNonExplicitConversionOperator
    operator T *() const { // NOLINT(*-explicit-constructor)
        return ptr_;
    }

    T &operator*() const {
        return *ptr_;
    }

    T *operator->() const {
        return ptr_;
    }

protected:
    T *ptr_;
    Deleter deleter_;
};


#endif
