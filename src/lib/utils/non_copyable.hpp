#ifndef SRC_LIB_UTILS_NON_COPYABLE_HPP
#define SRC_LIB_UTILS_NON_COPYABLE_HPP

class NonCopyable {
    // 参照メンバがあると implicit な代入演算子では代入できない
    const NonCopyable &selfRef_;

    // 実装しないことで、コピーはリンクエラーになる
    NonCopyable(const NonCopyable &);

protected:
    NonCopyable():
        selfRef_(*this) {}

    ~NonCopyable() {}
};

#endif
