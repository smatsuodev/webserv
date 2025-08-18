#ifndef EITHER_HPP
#define EITHER_HPP

#include <stdexcept>
#include <type_traits>

namespace types {
    template <class T>
    class Left {
    public:
        explicit Left(T val) : val_(val) {}

        // コピーコンストラクタではない
        // Either<T, R>(Left<U>) のコンストラクタで Left<U> -> Left<T> に変換するために必要
        template <class U>
        explicit Left(Left<U> &other) : val_(other.val()) {}

        Left(const Left &other) : val_(other.val_) {}

        ~Left() {}

        Left &operator=(const Left &other) {
            if (this != &other) {
                val_ = other.val_;
            }
            return *this;
        }

        T val() {
            return val_;
        }

        bool operator==(const Left &other) const {
            return val_ == other.val_;
        }

        bool operator!=(const Left &other) const {
            return val_ != other.val_;
        }

    private:
        T val_;
    };

    template <class T>
    class Right {
    public:
        explicit Right(T val) : val_(val) {}

        // コピーコンストラクタではない
        // Either<T, R>(Right<U>) のコンストラクタで Right<U> -> Right<T> に変換するために必要
        template <class U>
        explicit Right(Right<U> &other) : val_(other.val()) {}

        Right(const Right &other) : val_(other.val_) {}

        ~Right() {}

        Right &operator=(const Right &other) {
            if (this != &other) {
                val_ = other.val_;
            }
            return *this;
        }

        T val() {
            return val_;
        }

        bool operator==(const Right &other) const {
            return val_ == other.val_;
        }

        bool operator!=(const Right &other) const {
            return val_ != other.val_;
        }

    private:
        T val_;
    };
}

template <class T>
// ReSharper disable once CppInconsistentNaming
types::Left<T> Left(T val) {
    return types::Left<T>(val);
}

template <class T>
// ReSharper disable once CppInconsistentNaming
types::Right<T> Right(T val) {
    return types::Right<T>(val);
}

template <typename L, typename R>
class Either {
public:
    Either() : left_(NULL), right_(NULL) {}

    explicit Either(types::Left<L> *left) : left_(left), right_(NULL) {}

    explicit Either(types::Right<R> *right) : left_(NULL), right_(right) {}

    // Either<int, std::string> target = Left(0); みたいなことができるようにexplicitをつけない
    // NOLINTNEXTLINE(google-explicit-constructor)
    Either(types::Left<L> left) : left_(new types::Left<L>(left)), right_(NULL) {}

    /**
     * 以下のコードを書けるようにするために必要
     * (本来 Left<Option<int>> や Left<std::string> などの注釈が必要)
     * Either<Option<int>, E> r = Left(Some(42));
     * Either<std::string, E> r2 = Left("hello");
     */
    // U -> T に変換可能であるという制約を付けないと、IDE 上でエラーが出ない (コンパイル時に出る)
    template <class U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Either(types::Left<U> left) : left_(new types::Left<L>(left)), right_(NULL) {}

    // either.map(Some<L>) の型 Either<Some<L>, E> から Either<Option<L>, E> に変換するために必要
    template <class U>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Either(const Either<U, R> &other) {
        if (other.isLeft()) {
            // NOTE: U -> T に変換できる必要がある
            left_ = new types::Left<L>(other.unwrap());
            right_ = NULL;
        } else {
            right_ = new types::Right<R>(other.unwrapRight());
            left_ = NULL;
        }
    }

    // Either<int, std::string> target = Right<std::string>("right"); みたいなことができるようにexplicitをつけない
    // NOLINTNEXTLINE(google-explicit-constructor)
    Either(types::Right<R> right) : left_(NULL), right_(new types::Right<R>(right)) {}

    Either(const Either &other) {
        this->left_ = other.isLeft() ? new types::Left<L>(*other.left_) : NULL;
        this->right_ = other.isRight() ? new types::Right<R>(*other.right_) : NULL;
    }

    ~Either() {
        delete left_;
        delete right_;
    }

    Either &operator=(const Either &other) {
        if (this != &other) {
            if (other.isLeft()) {
                if (left_ == NULL) {
                    left_ = new types::Left<L>(*other.left_);
                } else {
                    *left_ = *other.left_;
                }
                delete right_;
                right_ = NULL;
            } else if (other.isRight()) {
                if (right_ == NULL) {
                    right_ = new types::Right<R>(*other.right_);
                } else {
                    *right_ = *other.right_;
                }
                delete left_;
                left_ = NULL;
            }
        }
        return *this;
    }

    bool operator==(const Either &other) const {
        if (isLeft() && other.isLeft()) {
            return *left_ == *other.left_;
        }
        if (isRight() && other.isRight()) {
            return *right_ == *other.right_;
        }
        return false;
    }

    bool operator!=(const Either &other) const {
        if (isLeft() && other.isLeft()) {
            return *left_ != *other.left_;
        }
        if (isRight() && other.isRight()) {
            return *right_ != *other.right_;
        }
        return true;
    }

    bool isLeft() const {
        return left_ != NULL;
    }

    bool isRight() const {
        return right_ != NULL;
    }

    L unwrapLeft() const {
        if (isRight()) {
            throw std::runtime_error("called `Either::unwrapLeft()` on an `Right` value");
        }
        return left_->val();
    }

    L unwrapLeftOr(L val) const {
        if (isLeft()) return left_->val();
        return val;
    }

    R unwrapRight() const {
        if (isLeft()) {
            throw std::runtime_error("called `Either::unwrapRight()` on an `Left` value");
        }
        return right_->val();
    }

    R unwrapRightOr(R val) const {
        if (isRight()) return right_->val();
        return val;
    }

private:
    types::Left<L> *left_;
    types::Right<R> *right_;
};

#endif // EITHER_HPP
