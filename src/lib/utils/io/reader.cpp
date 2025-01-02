#include "reader.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"
#include <cerrno>
#include <unistd.h>
#include <cstring>

namespace io {
    IReader::~IReader() {}

    FdReader::FdReader(AutoFd &fd) : fd_(fd), eof_(false) {}

    FdReader::~FdReader() {}

    FdReader::ReadResult FdReader::read(char *buf, const std::size_t nbyte) {
        /**
         * subject で read 直後の errno の確認は禁止されてる
         * EAGAIN とそれ以外の区別は EPOLLERR イベントで判定できる
         */
        const ssize_t bytesRead = ::read(fd_, buf, nbyte);
        if (bytesRead == -1) {
            return Err(error::kIOUnknown);
        }
        if (bytesRead == 0) {
            eof_ = true;
        }
        return Ok(static_cast<std::size_t>(bytesRead));
    }

    bool FdReader::eof() {
        return eof_;
    }
}
