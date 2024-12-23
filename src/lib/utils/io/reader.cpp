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
        const ssize_t bytesRead = ::read(fd_, buf, nbyte);
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
    Reader::Reader(io::IReader &reader, const std::size_t bufCapacity)
        : reader_(reader), bufCapacity_(bufCapacity), buf_(new char[bufCapacity_]), bufSize_(0), bufPos_(0) {}

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
            if (bytesRead == 0) {
                break;
            }
            result.append(buf, bytesRead);
        }

        return Ok(result);
    }

    Reader::PeekResult Reader::peek(const std::size_t nbyte) {
        // NOTE: fillBuf() の結果によっては、nbyte に足りない可能性がある
        TRY(this->ensureBufSize(nbyte));
        TRY(this->fillBuf());

        const std::size_t bytesToPeek = std::min(nbyte, this->unreadBufSize());
        return Ok(std::string(buf_ + bufPos_, bytesToPeek));
    }

    Reader::DiscardResult Reader::discard(const std::size_t nbyte) {
        // NOTE: fillBuf() の結果によっては、nbyte に足りない可能性がある
        TRY(this->ensureBufSize(nbyte));
        TRY(this->fillBuf());

        const std::size_t bytesToDiscard = std::min(nbyte, this->unreadBufSize());
        bufPos_ += bytesToDiscard;
        return Ok(bytesToDiscard);
    }

    std::size_t Reader::unreadBufSize() const {
        return this->bufSize_ - this->bufPos_;
    }

    Result<std::size_t, std::string> Reader::fillBuf() {
        const size_t remainingCapacity = this->bufCapacity_ - this->bufSize_;
        if (remainingCapacity == 0) {
            if (this->unreadBufSize() > 0) {
                // 既にバッファが埋まっていて、読み取っていないデータがある場合、何もしない
                return Ok<std::size_t>(0);
            }
            // バッファを入れ替える
            bufSize_ = TRY(reader_.read(buf_, bufCapacity_));
            bufPos_ = 0;
            return Ok(bufSize_);
        }

        // カーソルはそのままで、バッファを埋める
        const std::size_t bytesRead = TRY(reader_.read(buf_ + bufPos_, remainingCapacity));
        bufSize_ += bytesRead;
        return Ok(bytesRead);
    }

    Result<void, std::string> Reader::ensureBufSize(const std::size_t nbyte) {
        if (nbyte <= this->unreadBufSize()) {
            return Ok();
        }

        const size_t currentUnreadBufSize = this->unreadBufSize();
        if (bufCapacity_ < nbyte) {
            // バッファを拡張
            std::size_t newCapacity = bufCapacity_;
            while (newCapacity < nbyte) {
                newCapacity *= 2;
            }

            // バッファの確保、詰め替え
            char *newBuf = new char[newCapacity];
            std::memcpy(newBuf, buf_ + bufPos_, currentUnreadBufSize);
            delete[] buf_;

            buf_ = newBuf;
            bufCapacity_ = newCapacity;
            bufSize_ = currentUnreadBufSize;
            bufPos_ = 0;
        } else if (bufCapacity_ - bufPos_ < nbyte) {
            // capacity は足りているが、bufPos_ からの空きが足りない
            // 未読み取りのデータをバッファの先頭に移動
            std::memmove(buf_, buf_ + bufPos_, currentUnreadBufSize);
            bufSize_ = currentUnreadBufSize;
            bufPos_ = 0;
        }

        return Ok();
    }
}
