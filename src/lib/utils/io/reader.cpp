#include "reader.hpp"

#include "utils/logger.hpp"
#include "utils/string.hpp"
#include "utils/types/try.hpp"

#include <cerrno>
#include <unistd.h>
#include <cstring>

namespace io {
    IReader::~IReader() {}

    FdReader::FdReader(AutoFd &fd) : fd_(fd), eof_(false) {}

    FdReader::~FdReader() {}

    Result<std::size_t, std::string> FdReader::read(char *buf, const std::size_t nbyte) {
        const ssize_t bytesRead = ::read(fd_.get(), buf, nbyte);
        if (bytesRead == -1) {
            return Err(utils::format("failed to read: %s", std::strerror(errno)));
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

namespace bufio {
    Reader::Reader(io::IReader &reader)
        : reader_(reader), bufCapacity_(kDefaultBufSize), buf_(new char[bufCapacity_]), bufSize_(0), bufPos_(0) {}

    Reader::~Reader() {
        delete[] buf_;
    }

    Result<std::size_t, std::string> Reader::read(char *buf, const std::size_t nbyte) {
        if (buf == NULL) {
            return Err<std::string>("buf cannot be NULL");
        }
        if (nbyte == 0) {
            return Ok<std::size_t>(0);
        }

        std::size_t totalBytesRead = 0;
        const Result<int, std::string> r = Ok(1);
        while (totalBytesRead < nbyte) {
            if (this->unreadBufSize() == 0) {
                if (TRY(this->fillBuf()) == 0) {
                    // reader_ からこれ以上読み取れないので終了
                    break;
                }
            }

            // 内部バッファから引数の buf にコピー
            const std::size_t bytesToCopy = std::min(nbyte - totalBytesRead, this->unreadBufSize());
            std::memcpy(buf + totalBytesRead, buf_ + bufPos_, bytesToCopy);

            bufPos_ += bytesToCopy;
            totalBytesRead += bytesToCopy;
        }

        return Ok(totalBytesRead);
    }

    bool Reader::eof() {
        return this->reader_.eof() && this->unreadBufSize() == 0;
    }

    Reader::ReadUntilResult Reader::readUntil(const std::string &delimiter) {
        const size_t delimiterLen = delimiter.length();
        if (delimiterLen == 0) {
            return Err<std::string>("delimiter cannot be empty");
        }

        std::string result;

        /**
         * 読み込んだ文字列の末尾を保持する
         * 先頭の文字の削除は遅いが、delimiter は大抵短いので無視できる
         */
        std::string window;
        window.reserve(delimiterLen);

        while (true) {
            char ch;
            const ssize_t bytesRead = TRY(this->read(&ch, 1));
            if (bytesRead == 0) {
                // EOF に達した場合
                break;
            }

            result += ch;

            window += ch;
            if (window.length() > delimiterLen) {
                // 先頭の文字を削除してウィンドウサイズを維持
                window.erase(0, 1);
            }

            if (window == delimiter) {
                return Ok(result);
            }
        }

        // EOF に達しても delimiter が見つからなかった場合
        return Ok(result);
    }

    Reader::ReadAllResult Reader::readAll() {
        std::string result;

        while (true) {
            char buf[4096];
            const ssize_t bytesRead = TRY(this->read(buf, sizeof(buf)));

            result.append(buf, bytesRead);

            if (bytesRead < sizeof(buf)) {
                break;
            }
        }

        return Ok(result);
    }

    std::size_t Reader::unreadBufSize() const {
        return this->bufSize_ - this->bufPos_;
    }

    Result<std::size_t, std::string> Reader::fillBuf() {
        bufSize_ = TRY(reader_.read(buf_, bufCapacity_));
        bufPos_ = 0;
        return Ok(bufSize_);
    }
}
