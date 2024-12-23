#ifndef SRC_LIB_UTILS_TYPES_TRY_HPP
#define SRC_LIB_UTILS_TYPES_TRY_HPP

#include "result.hpp"
#include "option.hpp"

template <typename T, typename E>
types::Err<E> tryDefault(const Result<T, E> &res) {
    return Err(res.unwrapErr());
}

template <typename T>
types::None tryDefault(const Option<T> &) {
    return None;
}

/**
 * NOTE: TRY, TRY_OR の expr は副作用を起こしてはいけない
 * 展開によって複数回評価されてしまう
 * (副作用があっても冪等ならOK)
 */
#define TRY(expr) TRY_OR(expr, tryDefault(expr))

#define TRY_OR(expr, defaultValue)                                               \
    ({                                                                           \
        typeof(expr) e = (expr); /* NOLINT(*-unnecessary-copy-initialization) */ \
        if (!(e).canUnwrap()) return defaultValue;                               \
        (e).unwrap();                                                            \
    })

#endif
