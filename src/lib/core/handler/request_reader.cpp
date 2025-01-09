#include "request_reader.hpp"
#include "../../utils/string.hpp"
#include "../../http/request/request_parser.hpp"
#include "../../utils/types/try.hpp"
#include "../../utils/logger.hpp"

RequestReader::RequestReader(IConfigResolver &resolver)
    : resolver_(resolver), state_(kReadingRequestLine), contentLength_(None), clientMaxBodySize_(0), body_(None) {}

RequestReader::ReadRequestResult RequestReader::readRequest(ReadBuffer &readBuf) {
    LOG_DEBUG("start RequestReader::readRequest");

    const std::size_t bytesLoaded = TRY(readBuf.load());
    while (state_ != kDone && readBuf.size() > 0) {
        switch (state_) {
            // request-line CRLF
            case kReadingRequestLine: {
                LOG_DEBUG("read start-line");
                Option<std::string> result = TRY(this->getRequestLine(readBuf));
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
                Option<types::Unit> result = TRY(this->getHeaders(readBuf));
                if (result.isNone()) {
                    if (bytesLoaded == 0) {
                        LOG_WARN("incomplete headers");
                        return Err(error::kUnknown);
                    }
                    return Ok(None);
                }
                contentLength_ = TRY(this->getContentLength(headers_));
                // config から client_max_body_size を取得
                clientMaxBodySize_ = TRY(this->resolveClientMaxBodySize(headers_));
                if (contentLength_.isSome() && contentLength_.unwrap() > 0) {
                    const std::size_t len = contentLength_.unwrap();
                    if (len > clientMaxBodySize_) {
                        LOG_WARNF("Content-Length is too large: %zu", len);
                        return Err(error::kHttpPayloadTooLarge);
                    }
                    state_ = kReadingBody;
                } else {
                    state_ = kDone;
                }
                break;
            }

            // message-body
            case kReadingBody: {
                LOG_DEBUG("read message-body");
                const Option<types::Unit> result = TRY(this->getBody(readBuf, contentLength_.unwrap()));
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

RequestReader::GetLineResult RequestReader::getLine(ReadBuffer &readBuf) {
    const Option<std::string> maybeLine = readBuf.consumeUntil("\r\n");
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

Result<Option<std::string>, error::AppError> RequestReader::getRequestLine(ReadBuffer &readBuf) const {
    const Option<std::string> reqLine = TRY(this->getLine(readBuf));
    return Ok(reqLine);
}

Result<Option<types::Unit>, error::AppError> RequestReader::getHeaders(ReadBuffer &readBuf) {
    while (true) {
        const Option<std::string> line = TRY(this->getLine(readBuf));
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
            return Ok(Some(TRY(utils::stoul(field.second))));
        }
    }
    return Ok(None);
}

Result<std::size_t, error::AppError> RequestReader::resolveClientMaxBodySize(const RawHeaders &headers) const {
    // Host ヘッダーを探す
    Option<std::string> hostHeaderValue = None;
    for (RawHeaders::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        const http::RequestParser::HeaderField &field = TRY(http::RequestParser::parseHeaderFieldLine(*it));
        if (field.first == "Host") {
            hostHeaderValue = Some(field.second);
            break;
        }
    }
    if (hostHeaderValue.isNone()) {
        // Host ヘッダーは必須
        return Err(error::kParseUnknown);
    }

    const Option<config::ServerContext> config = resolver_.resolve(hostHeaderValue.unwrap());
    if (config.isNone()) {
        // accept しているので、通常は見つかるはず
        LOG_WARN("config not found");
        return Err(error::kUnknown);
    }

    return Ok(config.unwrap().getClientMaxBodySize());
}

Result<Option<types::Unit>, error::AppError>
RequestReader::getBody(ReadBuffer &readBuf, const std::size_t contentLength) {
    while (bodyBuf_.size() < contentLength) {
        const std::size_t want = contentLength - bodyBuf_.size();
        const std::string chunk = readBuf.consume(want);
        if (chunk.empty()) {
            return Ok(None);
        }
        bodyBuf_ += chunk;
    }

    body_ = Some(bodyBuf_);
    return Ok(Some(unit));
}

RequestReader::IConfigResolver::~IConfigResolver() {}

RequestReader::ConfigResolver::ConfigResolver(const VirtualServerResolver &vsResolver) : resolver_(vsResolver) {}

Option<config::ServerContext> RequestReader::ConfigResolver::resolve(const std::string &host) const {
    const Option<Ref<VirtualServer> > vs = resolver_.resolve(host);
    if (vs.isNone()) {
        return None;
    }
    return Some(vs.unwrap().get().getServerConfig());
}
