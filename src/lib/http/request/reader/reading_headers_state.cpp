#include "reading_headers_state.hpp"
#include "utils/types/try.hpp"
#include "./utils.hpp"
#include "reading_body_state.hpp"
#include "utils/logger.hpp"
#include "config/config.hpp"

http::ReadingHeadersState::ReadingHeadersState(RequestReader &reader) : reader_(reader) {}

Result<http::RequestReader::IState::HandleStatus, error::AppError>
http::ReadingHeadersState::handle(ReadBuffer &readBuf) {
    while (true) {
        const Option<std::string> line = TRY(getLine(readBuf));
        if (line.isNone()) {
            // バッファが足りない
            return Ok(HandleStatus::kSuspend);
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

    reader_.setHeaders(headers_);

    // TODO: Transfer-Encoding 対応
    const std::size_t clientMaxBodySize = TRY(getClientMaxBodySize(headers_));
    const Option<std::size_t> contentLength = TRY(getContentLength(headers_));
    if (contentLength.isSome() && contentLength.unwrap() > 0) {
        const std::size_t len = contentLength.unwrap();
        if (len > clientMaxBodySize) {
            LOG_WARN("content-length too large");
            return Err(error::kParseUnknown);
        }
        reader_.changeState(new ReadingBodyState(reader_, len));
    } else {
        // body がなければ request は読み終わり
        reader_.changeState(NULL);
    }

    return Ok(HandleStatus::kDone);
}

Result<std::size_t, error::AppError> http::ReadingHeadersState::getClientMaxBodySize(const RawHeaders &headers) const {
    const Option<std::string> host = TRY(getHeaderValue(headers, "Host"));
    if (host.isNone()) {
        // Host ヘッダーは必須
        return Err(error::kParseUnknown);
    }

    const Option<config::ServerContext> config = reader_.getConfigResolver().resolve(host.unwrap());
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
