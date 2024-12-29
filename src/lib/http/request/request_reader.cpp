#include "request_reader.hpp"
#include "utils/string.hpp"
#include "request_parser.hpp"
#include "utils/types/try.hpp"
#include "utils/logger.hpp"

RequestReader::RequestReader(bufio::Reader &reader)
    : reader_(reader), state_(kReadingRequestLine), contentLength_(None), bodyBuf_(NULL), bodyBufSize_(0), body_(None) {
}

RequestReader::~RequestReader() {
    delete[] bodyBuf_;
}

RequestReader::ReadRequestResult RequestReader::readRequest() {
    LOG_DEBUG("start RequestReader::readRequest");

    while (state_ != kDone) {
        switch (state_) {
            // request-line CRLF
            case kReadingRequestLine: {
                LOG_DEBUG("read start-line");
                TRY(this->readRequestLine());
                state_ = kReadingHeaders;
                break;
            }

            // *( field-line CRLF ) CRLF
            case kReadingHeaders: {
                LOG_DEBUG("read headers");
                TRY(this->readHeaders());
                state_ = contentLength_.isSome() ? kReadingBody : kDone;
                break;
            }

            // message-body
            case kReadingBody: {
                LOG_DEBUG("read message-body");
                TRY(this->readBody());
                state_ = kDone;
                break;
            }

            case kDone: {
                LOG_DEBUG("request has been read");
                break;
            }

            default: {
                LOG_DEBUGF("RequestReader::readRequest returned unexpected state");
                return Err(error::kUnknown);
            }
        }
    }

    return Ok(TRY(http::RequestParser::parseRequest(rawRequestLine_, headers_, body_.unwrapOr(""))));
}

Result<std::string, error::AppError> RequestReader::readLine() const {
    std::string requestLine = TRY(reader_.readUntil("\r\n"));
    if (!utils::endsWith(requestLine, "\r\n")) {
        return Err(error::kParseUnknown);
    }

    requestLine.erase(requestLine.size() - 2);
    return Ok(requestLine);
}

Result<void, error::AppError> RequestReader::readRequestLine() {
    const Result<std::string, error::AppError> result = this->readLine();
    if (result.isErr()) {
        if (result.unwrapErr() == error::kIOWouldBlock) {
            return Err(error::kIOWouldBlock);
        }
        LOG_DEBUG("start-line does not end with CRLF");
        return Err(error::kParseUnknown);
    }

    rawRequestLine_ = result.unwrap();
    return Ok();
}

Result<void, error::AppError> RequestReader::readHeaders() {
    while (true) {
        const Result<std::string, error::AppError> readResult = this->readLine();
        if (readResult.isErr()) {
            if (readResult.unwrapErr() == error::kIOWouldBlock) {
                return Err(error::kIOWouldBlock);
            }
            LOG_DEBUG("field-line does not end with CRLF");
            return Err(error::kParseUnknown);
        }

        /**
         * 空行 (\r\n) はヘッダーの終わり
         * this->readLine() は trim 済みなので、empty() かどうかを確認
         */
        std::string header = readResult.unwrap();
        if (header.empty()) {
            break;
        }
        headers_.push_back(header);

        // Content-Length ヘッダーの値を取得
        const http::RequestParser::HeaderField &field = TRY(http::RequestParser::parseHeaderFieldLine(header));
        if (field.first == "Content-Length") {
            // TODO: client_max_body_size より大きい値の場合はエラー
            contentLength_ = Some(TRY(utils::stoul(field.second)));
        }
    }

    return Ok();
}

Result<void, error::AppError> RequestReader::readBody() {
    const size_t &bodySize = contentLength_.unwrap();
    if (!bodyBuf_) {
        bodyBuf_ = new char[bodySize];
    }

    while (bodyBufSize_ < bodySize) {
        bodyBufSize_ += TRY(reader_.read(bodyBuf_ + bodyBufSize_, bodySize - bodyBufSize_));
    }

    body_ = Some(std::string(bodyBuf_, bodySize));
    return Ok();
}
