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

#ifdef __clang__
// typeof(expr) は式を評価しないので、全体として一回だけ expr が評価される
#define TRY(expr)                                                                  \
    ({                                                                             \
        __typeof(expr) e = (expr); /* NOLINT(*-unnecessary-copy-initialization) */ \
        if (!(e).canUnwrap()) return tryDefault(e);                                \
        (e).unwrap();                                                              \
    })

#define TRY_OR(expr, defaultValue)                                                 \
    ({                                                                             \
        __typeof(expr) e = (expr); /* NOLINT(*-unnecessary-copy-initialization) */ \
        if (!(e).canUnwrap()) return defaultValue;                                 \
        (e).unwrap();                                                              \
    })
#else
#define TRY(expr)                                                                \
    ({                                                                           \
        typeof(expr) e = (expr); /* NOLINT(*-unnecessary-copy-initialization) */ \
        if (!(e).canUnwrap()) return tryDefault(e);                              \
        (e).unwrap();                                                            \
    })

#define TRY_OR(expr, defaultValue)                                               \
    ({                                                                           \
        typeof(expr) e = (expr); /* NOLINT(*-unnecessary-copy-initialization) */ \
        if (!(e).canUnwrap()) return defaultValue;                               \
        (e).unwrap();                                                            \
    })
#endif


#endif
