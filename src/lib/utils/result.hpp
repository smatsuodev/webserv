#ifndef SRC_LIB_UTILS_RESULT_HPP
#define SRC_LIB_UTILS_RESULT_HPP

#include <cstddef>
#include <stdexcept>

template <class T, class E>
class Result;

namespace types {
    template <class T>
    class Ok {
    public:
        explicit Ok(T val) : val_(val) {}

        Ok(const Ok &other) : val_(other.val_) {}

        ~Ok() {}

        Ok &operator=(const Ok &other) {
            if (this != &other) {
                val_ = other.val_;
            }
            return *this;
        }

        T val() {
            return val_;
        }

        bool operator==(const Ok &other) const {
            return val_ == other.val_;
        }

        bool operator!=(const Ok &other) const {
            return val_ != other.val_;
        }

    private:
        T val_;
    };

    template <class E>
    class Err {
    public:
        explicit Err(E error) : error_(error) {}

        Err(const Err &other) : error_(other.error_) {}

        ~Err() {}

        Err &operator=(const Err &other) {
            if (this != &other) {
                error_ = other.error_;
            }
            return *this;
        }

        E error() {
            return error_;
        }

        bool operator==(const Err &other) const {
            return error_ == other.error_;
        }

        bool operator!=(const Err &other) const {
            return error_ != other.error_;
        }

    private:
        E error_;
    };
}

template <class T>
// ReSharper disable once CppInconsistentNaming
types::Ok<T> Ok(T val) {
    return types::Ok<T>(val);
}

template <class E>
// ReSharper disable once CppInconsistentNaming
types::Err<E> Err(E val) {
    return types::Err<E>(val);
}

template <class T, class E>
class Result {
public:
    Result() : ok_(NULL), err_(NULL) {}

    explicit Result(types::Ok<T> *ok) : ok_(ok), err_(NULL) {}

    explicit Result(types::Err<E> *err) : ok_(NULL), err_(err) {}

    // Result<int, std::string> target = Ok(0); みたいなことができるようにexplicitをつけない
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(types::Ok<T> ok) : ok_(new types::Ok<T>(ok)), err_(NULL) {}

    // Result<int, std::string> target = Err<std::string>("error"); みたいなことができるようにexplicitをつけない
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(types::Err<E> err) : ok_(NULL), err_(new types::Err<E>(err)) {}

    Result(const Result &other) {
        this->ok_ = other.isOk() ? new types::Ok<T>(*other.ok_) : NULL;
        this->err_ = other.isErr() ? new types::Err<E>(*other.err_) : NULL;
    }

    ~Result() {
        delete ok_;
        delete err_;
    }

    Result &operator=(const Result &other) {
        if (this != &other) {
            if (other.isOk()) {
                if (ok_ == NULL) {
                    ok_ = new types::Ok<T>(*other.ok_);
                } else {
                    *ok_ = *other.ok_;
                }
                err_ = NULL;
            } else if (other.isErr()) {
                if (err_ == NULL) {
                    err_ = new types::Err<E>(*other.err_);
                } else {
                    *err_ = *other.err_;
                }
                ok_ = NULL;
            }
        }
        return *this;
    }

    bool operator==(const Result &other) const {
        if (isOk() && other.isOk()) {
            return *ok_ == *other.ok_;
        }
        if (isErr() && other.isErr()) {
            return *err_ == *other.err_;
        }
        return false;
    }

    bool operator!=(const Result &other) const {
        if (isOk() && other.isOk()) {
            return *ok_ != *other.ok_;
        }
        if (isErr() && other.isErr()) {
            return *err_ != *other.err_;
        }
        return true;
    }

    bool isOk() const {
        return ok_ != NULL;
    }

    bool isErr() const {
        return err_ != NULL;
    }

    T unwrap() const {
        if (isErr()) {
            throw std::runtime_error("called `Result::unwrap()` on an `Err` value");
        }
        return ok_->val();
    }

    T unwrapOr(T val) const {
        if (isOk()) return ok_->val();
        return val;
    }

    E unwrapErr() const {
        if (isOk()) {
            throw std::runtime_error("called `Result::unwrapErr()` on an `Ok` value");
        }
        return err_->error();
    }

    bool canUnwrap() const {
        return isOk();
    }

private:
    types::Ok<T> *ok_;
    types::Err<E> *err_;
};

#define TRY(expr) TRY_OR(expr, Err((e).unwrapErr()))

#define TRY_OR(expr, defaultValue)                                                                                     \
    ({                                                                                                                 \
        typeof(expr) e = (expr);                                                                                       \
        if (!(e).canUnwrap()) return defaultValue;  \
    (e).unwrap();   \
})

#endif