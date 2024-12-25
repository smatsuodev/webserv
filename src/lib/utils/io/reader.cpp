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
        errno = 0;
        const ssize_t bytesRead = ::read(fd_, buf, nbyte);
        if (bytesRead == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return Err(error::kIOWouldBlock);
            }
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

namespace bufio {
    Reader::Reader(io::IReader &reader, const std::size_t bufCapacity)
        : reader_(reader), bufCapacity_(bufCapacity), buf_(new char[bufCapacity_]), bufSize_(0), bufPos_(0) {}

    Reader::~Reader() {
        delete[] buf_;
    }

    Reader::ReadResult Reader::read(char *buf, const std::size_t nbyte) {
        if (buf == NULL || nbyte == 0) {
            return Err(error::kValidationInvalidArgs);
        }

        std::size_t totalBytesRead = 0;
        while (totalBytesRead < nbyte) {
            if (this->unreadBufSize() == 0) {
                const Result<size_t, error::AppError> fillBufResult = this->fillBuf();
                if (fillBufResult.isErr()) {
                    const error::AppError err = fillBufResult.unwrapErr();
                    if (err == error::kIOWouldBlock) {
                        return Ok(totalBytesRead);
                    }

                    // 引数の buf に書き込まれたデータが失われないように、バッファに戻す
                    if (totalBytesRead > 0) {
                        this->prependBuf(buf, totalBytesRead);
                    }
                    return Err(err);
                }

                if (fillBufResult.unwrap() == 0) {
                    // reader_ からこれ以上読み込めない
                    break;
                }
            }

            // 内部バッファから引数の buf にコピー
            const std::size_t bytesCopied = this->consumeAndCopyBuf(buf + totalBytesRead, nbyte - totalBytesRead);
            totalBytesRead += bytesCopied;
        }

        return Ok(totalBytesRead);
    }

    bool Reader::eof() {
        return this->reader_.eof() && this->unreadBufSize() == 0;
    }

    Reader::ReadUntilResult Reader::readUntil(const std::string &delimiter) {
        const size_t delimiterLen = delimiter.length();
        if (delimiterLen == 0) {
            return Err(error::kValidationInvalidArgs);
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
            errno = 0;
            const ReadResult readResult = this->read(&ch, 1);
            /**
             * this->read だとバッファリングの都合で、読み込めたところまでの部分的な結果が返ってくる
             * 部分的な読込結果なのか、eof なのか区別するために、errno を確認する
             *
             * ただし、途中で errno がリセットがリセットされる可能性もあるので、あまりよくない
             */
            if (readResult.isErr() || (errno == EAGAIN || errno == EWOULDBLOCK)) {
                if (!result.empty()) {
                    this->prependBuf(result.data(), result.size());
                }
                if (readResult.isErr()) {
                    return Err(readResult.unwrapErr());
                }
                return Err(error::kIOWouldBlock);
            }
            if (readResult.unwrap() == 0) {
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
            std::size_t bytesRead;
            // readUntil と同じ理由で、read 先を使い分ける
            if (this->unreadBufSize() > 0) {
                bytesRead = this->consumeAndCopyBuf(buf, sizeof(buf));
            } else {
                const ReadResult readResult = reader_.read(buf, sizeof(buf));
                if (readResult.isErr()) {
                    // ここまで読み込んだ分をバッファに戻す
                    if (!result.empty()) {
                        this->prependBuf(result.data(), result.size());
                    }
                    return Err(readResult.unwrapErr());
                }
                bytesRead = readResult.unwrap();
            }

            if (bytesRead == 0) {
                break;
            }
            result.append(buf, bytesRead);
        }

        return Ok(result);
    }

    Reader::PeekResult Reader::peek(const std::size_t nbyte) {
        if (this->unreadBufSize() >= nbyte) {
            return Ok(std::string(buf_ + bufPos_, nbyte));
        }

        // NOTE: fillBuf() の結果によっては、nbyte に足りない可能性がある
        {
            const Result<void, error::AppError> r1 = this->ensureBufSize(nbyte);
            const Result<size_t, error::AppError> r2 = this->fillBuf();
            TRY(r1);
            TRY(r2);
        }

        const std::size_t bytesToPeek = std::min(nbyte, this->unreadBufSize());
        return Ok(std::string(buf_ + bufPos_, bytesToPeek));
    }

    Reader::DiscardResult Reader::discard(const std::size_t nbyte) {
        if (this->unreadBufSize() >= nbyte) {
            bufPos_ += nbyte;
            return Ok(nbyte);
        }

        // NOTE: fillBuf() の結果によっては、nbyte に足りない可能性がある
        {
            const Result<void, error::AppError> r1 = this->ensureBufSize(nbyte);
            const Result<size_t, error::AppError> r2 = this->fillBuf();
            TRY(r1);
            TRY(r2);
        }

        const std::size_t bytesToDiscard = std::min(nbyte, this->unreadBufSize());
        bufPos_ += bytesToDiscard;
        return Ok(bytesToDiscard);
    }

    std::size_t Reader::unreadBufSize() const {
        return this->bufSize_ - this->bufPos_;
    }

    Result<std::size_t, error::AppError> Reader::fillBuf() {
        const size_t remainingCapacity = this->bufCapacity_ - this->bufSize_;
        if (remainingCapacity == 0) {
            if (this->unreadBufSize() > 0) {
                // 既にバッファが埋まっていて、読み取っていないデータがある場合、何もしない
                return Ok<std::size_t>(0);
            }
            // バッファを入れ替える
            const ReadResult readResult = reader_.read(buf_, bufCapacity_);
            bufSize_ = TRY(readResult);
            bufPos_ = 0;
            return Ok(bufSize_);
        }

        // カーソルはそのままで、バッファを埋める
        const ReadResult readResult = reader_.read(buf_ + bufPos_, remainingCapacity);
        const std::size_t bytesRead = TRY(readResult);
        bufSize_ += bytesRead;
        return Ok(bytesRead);
    }

    Result<void, error::AppError> Reader::ensureBufSize(const std::size_t nbyte) {
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

    std::size_t Reader::consumeAndCopyBuf(char *buf, const std::size_t nbyte) {
        const size_t bytesToCopy = std::min(nbyte, this->unreadBufSize());
        if (bytesToCopy > 0) {
            std::memcpy(buf, buf_ + bufPos_, bytesToCopy);
            bufPos_ += bytesToCopy;
        }
        return bytesToCopy;
    }


    Result<void, error::AppError> Reader::prependBuf(const char *buf, const std::size_t nbyte) {
        const size_t newBufSize = this->unreadBufSize() + nbyte;
        {
            const Result<void, error::AppError> r = this->ensureBufSize(newBufSize);
            TRY(r);
        }

        if (bufSize_ == 0) {
            std::memcpy(buf_, buf, nbyte);
            bufSize_ += nbyte;
            return Ok();
        }

        if (bufPos_ < nbyte) {
            // bufPos_ >= nbyte を満たすように、既存のデータを後ろにずらす
            const size_t moveAmount = nbyte - bufPos_;
            std::memmove(buf_ + bufPos_ + moveAmount, buf_ + bufPos_, moveAmount);
            bufPos_ += moveAmount;
        }

        // この時点で bufPos_ >= nbyte を満たすはず
        // カーソルを nbyte 戻して、そこに書き込む
        bufPos_ -= nbyte;
        std::memcpy(buf_ + bufPos_, buf, nbyte);

        bufSize_ = newBufSize;

        return Ok();
    }
}
