#ifndef SRC_LIB_UTILS_TYPES_EITHER_HPP
#define SRC_LIB_UTILS_TYPES_EITHER_HPP

template <typename L, typename R>
class Either {
public:
    bool isLeft() const;
    bool isRight() const;
    L getLeft() const;
    R getRight() const;
};

template <typename L, typename R>
Either<L, R> Left(const L &value);

template <typename L, typename R>
Either<L, R> Right(const R &value);

#endif
