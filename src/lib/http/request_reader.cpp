#include "request_reader.hpp"
#include "utils/string.hpp"
#include "request_parser.hpp"
#include "utils/types/try.hpp"
#include "utils/logger.hpp"

RequestReader::RequestReader(bufio::Reader &reader)
    : reader_(reader), rawRequestLine_(None), headersParsed_(false), contentLength_(None), bodyBuf_(NULL),
      bodyBufSize_(0), body_(None) {}

RequestReader::~RequestReader() {
    delete[] bodyBuf_;
}

RequestReader::ReadRequestResult RequestReader::readRequest() {
    LOG_DEBUG("start RequestReader::readRequest");

    // request-line CRLF
    if (rawRequestLine_.isNone()) {
        LOG_DEBUG("read start-line");
        const Result<std::string, error::AppError> result = this->readLine();
        if (result.isErr()) {
            if (result.unwrapErr() == error::kIOWouldBlock) {
                return Err(error::kIOWouldBlock);
            }
            LOG_DEBUG("start-line does not end with CRLF");
            return Err(error::kParseUnknown);
        }
        rawRequestLine_ = Some(result.unwrap());
    }

    // *( field-line CRLF ) CRLF
    if (!headersParsed_) {
        LOG_DEBUG("read headers");
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
                headersParsed_ = true;
                break;
            }
            headers_.push_back(header);

            // Content-Length ヘッダーの値を取得
            const http::RequestParser::ParseHeaderFieldLineResult parseHeaderResult =
                http::RequestParser::parseHeaderFieldLine(header);
            const http::RequestParser::HeaderField &field = TRY(parseHeaderResult);
            if (field.first == "Content-Length") {
                // TODO: client_max_body_size より大きい値の場合はエラー
                const utils::StoulResult r = utils::stoul(field.second);
                contentLength_ = Some(TRY(r));
            }
        }
    }

    if (body_.isNone() && contentLength_.isSome()) {
        // message-body
        LOG_DEBUG("read message-body");

        const size_t &bodySize = contentLength_.unwrap();
        if (!bodyBuf_) {
            bodyBuf_ = new char[bodySize];
        }

        while (bodyBufSize_ < bodySize) {
            const bufio::Reader::ReadResult readResult = reader_.read(bodyBuf_ + bodyBufSize_, bodySize - bodyBufSize_);
            bodyBufSize_ += TRY(readResult);
        }

        body_ = Some(std::string(bodyBuf_, bodySize));
    }

    LOG_DEBUG("request has been read");

    const http::RequestParser::ParseResult parseResult =
        http::RequestParser::parseRequest(rawRequestLine_.unwrap(), headers_, body_.unwrapOr(""));

    return Ok(TRY(parseResult));
}

Result<std::string, error::AppError> RequestReader::readLine() const {
    const bufio::Reader::ReadUntilResult readResult = reader_.readUntil("\r\n");
    std::string requestLine = TRY(readResult);
    if (!utils::endsWith(requestLine, "\r\n")) {
        return Err(error::kParseUnknown);
    }

    requestLine.erase(requestLine.size() - 2);
    return Ok(requestLine);
}
