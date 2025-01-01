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
                Option<std::string> result = TRY(getRequestLine(readBuf_));
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
                Option<RawHeaders> result = TRY(getHeaders(readBuf_));
                if (result.isNone()) {
                    if (bytesLoaded == 0) {
                        LOG_WARN("incomplete headers");
                        return Err(error::kUnknown);
                    }
                    return Ok(None);
                }
                headers_ = result.unwrap();
                contentLength_ = TRY(getContentLength(result.unwrap()));
                state_ = contentLength_.isSome() ? kReadingBody : kDone;
                break;
            }

            // message-body
            case kReadingBody: {
                LOG_DEBUG("read message-body");
                body_ = TRY(getBody(readBuf_, contentLength_.unwrap()));
                if (body_.isNone()) {
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
        LOG_WARN("incomplete request");
        return Err(error::kUnknown);
    }

    const http::Request req = TRY(http::RequestParser::parseRequest(requestLine_, headers_, body_.unwrapOr("")));
    return Ok(Some(req));
}

RequestReader::GetLineResult RequestReader::getLine(ReadBuffer &readBuf) {
    const Option<std::string> maybeLine = TRY(readBuf.consumeUntil("\r\n"));
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

Result<Option<std::string>, error::AppError> RequestReader::getRequestLine(ReadBuffer &readBuf) {
    const Option<std::string> reqLine = TRY(getLine(readBuf));
    return Ok(reqLine);
}

Result<Option<RequestReader::RawHeaders>, error::AppError> RequestReader::getHeaders(ReadBuffer &readBuf) {
    static RawHeaders headers;

    while (true) {
        const Option<std::string> line = TRY(getLine(readBuf));
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
        headers.push_back(header);
    }

    return Ok(Some(headers));
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

Result<Option<std::string>, error::AppError> RequestReader::getBody(ReadBuffer &readBuf,
                                                                    const std::size_t contentLength) {
    static std::string body;

    while (body.size() < contentLength) {
        const std::size_t want = contentLength - body.size();
        const std::string chunk = TRY(readBuf.consume(want));
        body += chunk;
    }

    return Ok(Some(body));
}
