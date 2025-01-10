#include "request_reader.hpp"
#include "reading_request_line_state.hpp"
#include "http/request/request_parser.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

http::IConfigResolver::~IConfigResolver() {}

http::RequestReader::IState::~IState() {}

http::RequestReader::RequestReader(IConfigResolver &resolver) : resolver_(resolver) {
    state_ = new ReadingRequestLineState(*this);
}

http::RequestReader::~RequestReader() {
    delete state_;
}

http::RequestReader::ReadRequestResult http::RequestReader::readRequest(ReadBuffer &readBuf) const {
    LOG_DEBUG("start RequestReader::readRequest");

    const std::size_t bytesLoaded = TRY(readBuf.load());
    // ReadingBodyState などが state を NULL に遷移させる
    while (state_ != NULL) {
        const Result<IState::HandleStatus, error::AppError> result = state_->handle(readBuf);
        if (result.isErr()) {
            return Err(result.unwrapErr());
        }

        const IState::HandleStatus status = result.unwrap();
        if (status == IState::kSuspend) {
            // バッファが足りない
            if (bytesLoaded == 0) {
                // eof なので、リトライしても解決しない
                LOG_WARN("incomplete request");
                return Err(error::kUnknown);
            }
            // リトライすればバッファにデータが追加される可能性がある
            return Ok(None);
        }
    }

    return Ok(Some(TRY(RequestParser::parseRequest(requestLine_, headers_, body_))));
}

void http::RequestReader::changeState(IState *state) {
    delete state_;
    state_ = state;
}

http::IConfigResolver &http::RequestReader::getConfigResolver() const {
    return resolver_;
}

void http::RequestReader::setRequestLine(const std::string &requestLine) {
    requestLine_ = requestLine;
}

void http::RequestReader::setHeaders(const RawHeaders &headers) {
    headers_ = headers;
}

void http::RequestReader::setBody(const std::string &body) {
    body_ = body;
}
