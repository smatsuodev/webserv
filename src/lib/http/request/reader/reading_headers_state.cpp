#include "reading_headers_state.hpp"
#include "utils/types/try.hpp"
#include "./utils.hpp"
#include "reading_body_state.hpp"
#include "reading_chunked_body_state.hpp"
#include "utils/logger.hpp"
#include "config/config.hpp"

http::ReadingHeadersState::ReadingHeadersState(RequestReader::ReadContext &ctx) : ctx_(ctx) {}

Result<http::RequestReader::IState::HandleStatus, error::AppError> http::ReadingHeadersState::handle(ReadBuffer &readBuf
) {
    while (true) {
        const Option<std::string> line = TRY(getLine(readBuf));
        if (line.isNone()) {
            // バッファが足りない
            return Ok(kSuspend);
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

    ctx_.setHeaders(headers_);
    ctx_.changeState(TRY(this->nextState()));

    return Ok(kHandleDone);
}

Result<http::RequestReader::IState *, error::AppError> http::ReadingHeadersState::nextState() const {
    const std::size_t clientMaxBodySize = TRY(getClientMaxBodySize(headers_));
    const Option<std::size_t> contentLength = TRY(getContentLength(headers_));
    const bool isChunked = TRY(ReadingHeadersState::isChunked(headers_));

    if (contentLength.isSome() && isChunked) {
        // Content-Length と Transfer-Encoding は一方だけ必要
        LOG_WARN("both Content-Length and Transfer-Encoding are set");
        return Err(error::kParseUnknown);
    }

    if (isChunked) {
        return Ok(new ReadingChunkedBodyState(ctx_, clientMaxBodySize));
    }

    if (contentLength.isSome() && contentLength.unwrap() > 0) {
        const std::size_t len = contentLength.unwrap();
        if (len > clientMaxBodySize) {
            LOG_WARN("content-length too large");
            return Err(error::kHttpPayloadTooLarge);
        }
        return Ok(new ReadingBodyState(ctx_, len));
    }

    // body がなければ request は読み終わり
    return Ok<IState *>(NULL);
}

Result<std::size_t, error::AppError> http::ReadingHeadersState::getClientMaxBodySize(const RawHeaders &headers) const {
    const Option<std::string> host = TRY(getHeaderValue(headers, "Host"));
    if (host.isNone()) {
        // Host ヘッダーは必須
        return Err(error::kParseUnknown);
    }

    const Option<config::ServerContext> config = ctx_.getConfigResolver().resolve(host.unwrap());
    if (config.isNone()) {
        // accept しているので、通常は見つかるはず
        LOG_WARN("config not found");
        return Err(error::kUnknown);
    }

    return Ok(config.unwrap().getClientMaxBodySize());
}

Result<Option<std::size_t>, error::AppError> http::ReadingHeadersState::getContentLength(const RawHeaders &headers) {
    const Option<std::string> contentLength = TRY(getHeaderValue(headers, "Content-Length"));
    if (contentLength.isNone()) {
        return Ok(None);
    }
    return Ok(Some(TRY(utils::stoul(contentLength.unwrap()))));
}

Result<bool, error::AppError> http::ReadingHeadersState::isChunked(const RawHeaders &headers) {
    const Option<std::string> transferEncoding = TRY(getHeaderValue(headers, "Transfer-Encoding"));
    if (transferEncoding.isNone()) {
        return Ok(false);
    }

    // chunked 以外は対応しない
    if (transferEncoding.unwrap() != "chunked") {
        // TODO: Not Implemented, Bad Request など、適切なレスポンスを返せるようにする
        LOG_DEBUG("unsupported Transfer-Encoding");
        return Err(error::kParseUnknown);
    }
    return Ok(true);
}
