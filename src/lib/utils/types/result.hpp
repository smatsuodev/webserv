#ifndef SRC_LIB_UTILS_TYPES_RESULT_HPP
#define SRC_LIB_UTILS_TYPES_RESULT_HPP

#include <cstddef>
#include <stdexcept>

template <class T>
class Option;

template <class T, class E>
class Result;

namespace types {
    template <class T>
    class Some;

    template <class E>
    class Err;
}

namespace types {
    template <class T>
    class Ok {
    public:
        explicit Ok(T val) : val_(val) {}

        // コピーコンストラクタではない
        // Result<T, E>(Ok<U>) のコンストラクタで Ok<U> -> Ok<T> に変換するために必要
        // TODO: 提出時に消す
        template <class U, typename = typename std::enable_if<std::is_convertible<U, T>::value>::type>
        explicit Ok(Ok<U> &other) : val_(other.val()) {}

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

    template <>
    class Ok<void> {
    public:
        explicit Ok() {}

        Ok(const Ok &) {}

        ~Ok() {}

        Ok &operator=(const Ok &) {
            return *this;
        }

        // ReSharper disable once CppMemberFunctionMayBeStatic
        void val() {}

        bool operator==(const Ok &) const {
            return true;
        }

        bool operator!=(const Ok &) const {
            return false;
        }
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

// ReSharper disable once CppInconsistentNaming
types::Ok<void> Ok();

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

    /**
     * 以下のコードを書けるようにするために必要
     * (本来 Ok<Option<int>> や Ok<std::string> などの注釈が必要)
     * Result<Option<int>, E> r = Ok(Some(42));
     * Result<std::string, E> r2 = Ok("hello");
     */
    // U -> T に変換可能であるという制約を付けないと、IDE 上でエラーが出ない (コンパイル時に出る)
    // TODO: std::is_convertible などは C++11 以降なので、提出時に消す
    template <class U, typename = typename std::enable_if<std::is_convertible<U, T>::value>::type>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(types::Ok<U> ok) : ok_(new types::Ok<T>(ok)), err_(NULL) {}

    // result.map(Some<T>) の型 Result<Some<T>, E> から Result<Option<T>, E> に変換するために必要
    // TODO: 提出時に消す
    template <class U, typename = typename std::enable_if<std::is_convertible<U, T>::value>::type>
    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(const Result<U, E> &other) {
        if (other.isOk()) {
            // NOTE: U -> T に変換できる必要がある
            ok_ = new types::Ok<T>(other.unwrap());
            err_ = NULL;
        } else {
            err_ = new types::Err<E>(other.unwrapErr());
            ok_ = NULL;
        }
    }

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
                delete err_;
                err_ = NULL;
            } else if (other.isErr()) {
                if (err_ == NULL) {
                    err_ = new types::Err<E>(*other.err_);
                } else {
                    *err_ = *other.err_;
                }
                delete ok_;
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

    template <typename U>
    Result<U, E> andThen(Result<U, E> (*func)(T)) const {
        if (isOk()) {
            return func(ok_->val());
        }
        return Err(err_->error());
    }

    template <typename U>
    Result<U, E> map(U (*func)(T)) const {
        if (isOk()) {
            return Ok(func(ok_->val()));
        }
        return Err(err_->error());
    }

    template <typename F>
    Result<T, F> mapErr(F (*func)(E)) const {
        if (isOk()) {
            return Ok(ok_->val());
        }
        return Err(func(err_->error()));
    }

private:
    types::Ok<T> *ok_;
    types::Err<E> *err_;
};

template <class E>
class Result<void, E> {
public:
    Result() : err_(NULL) {}

    explicit Result(types::Ok<void> *) : err_(NULL) {}

    explicit Result(types::Err<E> *err) : err_(err) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(const types::Ok<void> &) : err_(NULL) {}

    // NOLINTNEXTLINE(google-explicit-constructor)
    Result(types::Err<E> err) : err_(new types::Err<E>(err)) {}

    Result(const Result &other) {
        this->err_ = other.isErr() ? new types::Err<E>(*other.err_) : NULL;
    }

    ~Result() {
        delete err_;
    }

    Result &operator=(const Result &other) {
        if (this != &other) {
            if (other.isOk()) {
                err_ = NULL;
            } else if (other.isErr()) {
                if (err_ == NULL) {
                    err_ = new types::Err<E>(*other.err_);
                } else {
                    *err_ = *other.err_;
                }
            }
        }
        return *this;
    }

    bool operator==(const Result &other) const {
        if (isOk() && other.isOk()) {
            return true;
        }
        if (isErr() && other.isErr()) {
            return *err_ == *other.err_;
        }
        return false;
    }

    bool operator!=(const Result &other) const {
        if (isOk() && other.isOk()) {
            return false;
        }
        if (isErr() && other.isErr()) {
            return *err_ != *other.err_;
        }
        return true;
    }

    bool isOk() const {
        return err_ == NULL;
    }

    bool isErr() const {
        return err_ != NULL;
    }

    void unwrap() const {
        if (isErr()) {
            throw std::runtime_error("called `Result::unwrap()` on an `Err` value");
        }
    }

    // unwrapOr は意味のある定義にならないので実装しない

    E unwrapErr() const {
        if (isOk()) {
            throw std::runtime_error("called `Result::unwrapErr()` on an `Ok` value");
        }
        return err_->error();
    }

    bool canUnwrap() const {
        return isOk();
    }

    template <typename U>
    Result<U, E> andThen(Result<U, E> (*func)()) {
        if (isOk()) {
            return func();
        }
        return Err(err_->error());
    }

    template <typename F>
    Result<void, F> mapErr(F (*func)(E)) {
        if (isOk()) {
            return Ok();
        }
        return Err(func(err_->error()));
    }

private:
    types::Err<E> *err_;
};

#endif
