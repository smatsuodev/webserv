#ifndef SRC_LIB_UTILS_IO_READER_HPP
#define SRC_LIB_UTILS_IO_READER_HPP

#include "../types/result.hpp"
#include "../types/option.hpp"
#include "utils/auto_fd.hpp"
#include "utils/types/error.hpp"

namespace io {
    class IReader {
    public:
        virtual ~IReader();

        typedef Result<std::size_t, error::AppError> ReadResult;
        virtual ReadResult read(char *buf, std::size_t nbyte) = 0;
        virtual bool eof() = 0;
    };

    class FdReader : public IReader, NonCopyable {
    public:
        // 所有権を奪わないことを明示するため、AutoFd の参照を受け取る
        explicit FdReader(AutoFd &fd);
        virtual ~FdReader();

        virtual ReadResult read(char *buf, std::size_t nbyte);
        virtual bool eof();

    private:
        AutoFd &fd_;
        bool eof_;
    };
}

#endif
