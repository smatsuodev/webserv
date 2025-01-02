#include "error.hpp"

namespace error {
    AppError recover(AppError) {
        return kRecoverable;
    }

    // ReSharper disable once CppInconsistentNaming
    AppError recoverIOError(const AppError err) {
        if (err == kIOUnknown) {
            return kRecoverable;
        }
        return err;
    }
}
