#include "request_reader.hpp"
#include "utils/string.hpp"
#include "request_parser.hpp"
#include "utils/types/try.hpp"
#include "utils/logger.hpp"

RequestReader::RequestReader(ReadBuffer &readBuf)
    : readBuf_(readBuf), state_(kReadingRequestLine), contentLength_(None), body_(None) {}

RequestReader::ReadRequestResult RequestReader::readRequest() {
    LOG_DEBUG("start RequestReader::readRequest");

    const std::size_t bytesLoaded = TRY(readBuf_.load());

    while (state_ != kDone && readBuf_.size() > 0) {
        switch (state_) {
            // request-line CRLF
            case kReadingRequestLine: {
                LOG_DEBUG("read start-line");
                Option<std::string> result = TRY(this->getRequestLine());
                if (result.isNone()) {
                    if (bytesLoaded == 0) {
                        LOG_WARN("incomplete request-line");
                        return Err(error::kUnknown);
                    }
                    return Ok(None);
                }
                requestLine_ = result.unwrap();
                state_ = kReadingHeaders;
                break;
            }

            // *( field-line CRLF ) CRLF
            case kReadingHeaders: {
                LOG_DEBUG("read headers");
                Option<types::Unit> result = TRY(this->getHeaders());
                if (result.isNone()) {
                    if (bytesLoaded == 0) {
                        LOG_WARN("incomplete headers");
                        return Err(error::kUnknown);
                    }
                    return Ok(None);
                }
                contentLength_ = TRY(this->getContentLength(headers_));
                if (contentLength_.isSome() && contentLength_.unwrap() > 0) {
                    state_ = kReadingBody;
                } else {
                    state_ = kDone;
                }
                break;
            }

            // message-body
            case kReadingBody: {
                LOG_DEBUG("read message-body");
                const Option<types::Unit> result = TRY(this->getBody(contentLength_.unwrap()));
                if (result.isNone()) {
                    if (bytesLoaded == 0) {
                        LOG_WARN("incomplete body");
                        return Err(error::kUnknown);
                    }
                    return Ok(None);
                }
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

    if (state_ != kDone) {
        // バッファが足りない
        return Ok(None);
    }

    const http::Request req = TRY(http::RequestParser::parseRequest(requestLine_, headers_, body_.unwrapOr("")));
    return Ok(Some(req));
}

RequestReader::GetLineResult RequestReader::getLine() const {
    const Option<std::string> maybeLine = readBuf_.consumeUntil("\r\n");
    if (maybeLine.isNone()) {
        return Ok(None);
    }

    std::string line = maybeLine.unwrap();
    if (!utils::endsWith(line, "\r\n")) {
        return Err(error::kParseUnknown);
    }

    line.erase(line.size() - 2);
    return Ok(Some(line));
}

Result<Option<std::string>, error::AppError> RequestReader::getRequestLine() const {
    const Option<std::string> reqLine = TRY(this->getLine());
    return Ok(reqLine);
}

Result<Option<types::Unit>, error::AppError> RequestReader::getHeaders() {
    while (true) {
        const Option<std::string> line = TRY(this->getLine());
        if (line.isNone()) {
            // バッファが足りない
            return Ok(None);
        }

        /**
         * 空行 (\r\n) はヘッダーの終わり
         * this->readLine() は trim 済みなので、empty() かどうかを確認
         */
        std::string header = line.unwrap();
        if (header.empty()) {
            break;
        }
        headers_.push_back(header);
    }

    return Ok(Some(unit));
}

Result<Option<size_t>, error::AppError> RequestReader::getContentLength(const RawHeaders &headers) {
    for (RawHeaders::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        const http::RequestParser::HeaderField &field = TRY(http::RequestParser::parseHeaderFieldLine(*it));
        if (field.first == "Content-Length") {
            // TODO: client_max_body_size より大きい値の場合はエラー
            return Ok(Some(TRY(utils::stoul(field.second))));
        }
    }
    return Ok(None);
}

Result<Option<types::Unit>, error::AppError> RequestReader::getBody(const std::size_t contentLength) {
    while (bodyBuf_.size() < contentLength) {
        const std::size_t want = contentLength - bodyBuf_.size();
        const std::string chunk = readBuf_.consume(want);
        if (chunk.empty()) {
            return Ok(None);
        }
        bodyBuf_ += chunk;
    }

    body_ = Some(bodyBuf_);
    return Ok(Some(unit));
}
