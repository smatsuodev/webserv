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

        TRY(this->ensureBufSize(nbyte));
        if (this->unreadBufSize() < nbyte) {
            const size_t bytesRead = TRY(this->fillBuf());
            if (bytesRead == 0) {
                // eof に達した場合、nbyte に満たなくても結果を返す
                return Ok(this->consumeAndCopyBuf(buf, nbyte));
            }
        }

        // nbyte が大きい場合、1 回の read では足りない可能性がある
        if (this->unreadBufSize() < nbyte) {
            // NOTE: エラーではないので、本当は None を返したい
            return Err(error::kUnknown);
        }

        return Ok(this->consumeAndCopyBuf(buf, nbyte));
    }

    bool Reader::eof() {
        return this->reader_.eof() && this->unreadBufSize() == 0;
    }

    Reader::ReadUntilResult Reader::readUntil(const std::string &delimiter) {
        const size_t delimiterLen = delimiter.length();
        if (delimiterLen == 0) {
            return Err(error::kValidationInvalidArgs);
        }

        Option<char *> searchResult = utils::strnstr(buf_ + bufPos_, delimiter.c_str(), this->unreadBufSize());
        if (searchResult.isNone()) {
            if (this->isBufFull()) {
                // buf を埋めても delimiter が見つからないので、buf を拡張する
                TRY(this->ensureBufSize(bufCapacity_ * 2));
            }
            const std::size_t bytesRead = TRY(this->fillBuf());
            if (bytesRead == 0) {
                // eof の場合、delimiter が見つからなくても結果を返す
                return Ok<Option<std::string> >(Some(this->consumeAll()));
            }

            // 再検索
            searchResult = utils::strnstr(buf_ + bufPos_, delimiter.c_str(), this->unreadBufSize());
        }

        if (searchResult.isNone()) {
            // 再検索しても存在しない
            return Ok<Option<std::string> >(None);
        }

        // delimiter が見つかった
        const char *delimEnd = searchResult.unwrap() + delimiterLen; // delimiter の終わりの次を指す
        const std::size_t len = delimEnd - (buf_ + bufPos_);
        return Ok<Option<std::string> >(Some(this->consume(len)));
    }

    Reader::ReadAllResult Reader::readAll() {
        const std::size_t bytesRead = TRY(this->fillBuf());
        if (bytesRead == 0) {
            // eof に達した
            return Ok<Option<std::string> >(Some(this->consumeAll()));
        }

        // まだ読み取り可能なデータがある
        return Ok<Option<std::string> >(None);
    }

    Reader::PeekResult Reader::peek(const std::size_t nbyte) {
        if (this->unreadBufSize() >= nbyte) {
            return Ok(std::string(buf_ + bufPos_, nbyte));
        }

        // NOTE: fillBuf() の結果によっては、nbyte に足りない可能性がある
        TRY(this->ensureBufSize(nbyte));
        TRY(this->fillBuf());

        const std::size_t bytesToPeek = std::min(nbyte, this->unreadBufSize());
        return Ok(std::string(buf_ + bufPos_, bytesToPeek));
    }

    Reader::DiscardResult Reader::discard(const std::size_t nbyte) {
        if (this->unreadBufSize() >= nbyte) {
            bufPos_ += nbyte;
            return Ok(nbyte);
        }

        // NOTE: fillBuf() の結果によっては、nbyte に足りない可能性がある
        TRY(this->ensureBufSize(nbyte));
        TRY(this->fillBuf());

        const std::size_t bytesToDiscard = std::min(nbyte, this->unreadBufSize());
        bufPos_ += bytesToDiscard;
        return Ok(bytesToDiscard);
    }

    bool Reader::isBufFull() const {
        return bufSize_ == bufCapacity_;
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
            bufSize_ = TRY(reader_.read(buf_, bufCapacity_));
            bufPos_ = 0;
            return Ok(bufSize_);
        }

        // カーソルはそのままで、バッファを埋める
        const std::size_t bytesRead = TRY(reader_.read(buf_ + bufPos_, remainingCapacity));
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
        const std::size_t bytesToCopy = std::min(nbyte, this->unreadBufSize());
        if (bytesToCopy > 0) {
            std::memcpy(buf, buf_ + bufPos_, bytesToCopy);
            bufPos_ += bytesToCopy;
        }
        return bytesToCopy;
    }

    std::string Reader::consume(const std::size_t nbyte) {
        const std::size_t bytesToConsume = std::min(nbyte, this->unreadBufSize());
        const std::string result(buf_ + bufPos_, bytesToConsume);
        bufPos_ += bytesToConsume;
        return result;
    }

    std::string Reader::consumeAll() {
        return this->consume(this->unreadBufSize());
    }

    Result<void, error::AppError> Reader::prependBuf(const char *buf, const std::size_t nbyte) {
        const size_t newBufSize = this->unreadBufSize() + nbyte;
        TRY(this->ensureBufSize(newBufSize));

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
