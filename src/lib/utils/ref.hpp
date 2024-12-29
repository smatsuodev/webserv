#ifndef SRC_LIB_UTILS_REF_HPP
#define SRC_LIB_UTILS_REF_HPP

// C++11 の std::reference_wrapper を再現
template <typename T>
class Ref {
public:
    typedef T Type;

    explicit Ref(T &ref) : ptr_(&ref) {}
    Ref(const Ref &ref) : ptr_(ref.ptr_) {}

    ~Ref() {
        // あくまで reference なので、delete しない
    }

    Ref &operator=(const Ref &ref) {
        if (this != &ref) {
            ptr_ = ref.ptr_;
        }
        return *this;
    }

    // int &ref = Ref<int>(x); と書けるようにするため、explicit を付けない
    operator T &() const { // NOLINT(*-explicit-constructor)
        return *ptr_;
    }

    bool operator==(const Ref &ref) const {
        return ptr_ == ref.ptr_;
    }

    T &get() const {
        return *ptr_;
    }

private:
    T *ptr_;
};

#endif
