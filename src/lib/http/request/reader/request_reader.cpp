#include "request_reader.hpp"
#include "reading_request_line_state.hpp"
#include "http/request/request_parser.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

http::RequestReader::RequestReader(IConfigResolver &resolver) : readCtx_(resolver) {
    readCtx_.changeState(new ReadingRequestLineState(readCtx_));
}

http::RequestReader::ReadRequestResult http::RequestReader::readRequest(ReadBuffer &readBuf) const {
    LOG_DEBUG("start RequestReader::readRequest");

    const std::size_t bytesLoaded = TRY(readBuf.load());
    // ReadingBodyState などが state を NULL に遷移させる
    while (readCtx_.getState() != NULL) {
        const Result<IState::HandleStatus, error::AppError> result = readCtx_.handle(readBuf);
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

    return Ok(Some(TRY(RequestParser::parseRequest(readCtx_.getRequestLine(), readCtx_.getHeaders(), readCtx_.getBody())
    )));
}

http::IConfigResolver::~IConfigResolver() {}

http::RequestReader::IState::~IState() {}

http::RequestReader::ReadContext::ReadContext(IConfigResolver &resolver, IState *initialState)
    : state_(initialState), resolver_(resolver) {}

http::RequestReader::ReadContext::~ReadContext() {
    delete state_;
}

Result<http::RequestReader::IState::HandleStatus, error::AppError>
http::RequestReader::ReadContext::handle(ReadBuffer &readBuf) const {
    if (state_ == NULL) {
        // 終了状態で呼び出したら、何もしない
        return Ok(IState::kHandleDone);
    }
    return state_->handle(readBuf);
}

const http::RequestReader::IState *http::RequestReader::ReadContext::getState() const {
    return state_;
}

void http::RequestReader::ReadContext::changeState(IState *state) {
    delete state_;
    state_ = state;
}

http::IConfigResolver &http::RequestReader::ReadContext::getConfigResolver() const {
    return resolver_;
}

const std::string &http::RequestReader::ReadContext::getRequestLine() const {
    return requestLine_;
}

const http::RawHeaders &http::RequestReader::ReadContext::getHeaders() const {
    return headers_;
}

const std::string &http::RequestReader::ReadContext::getBody() const {
    return body_;
}

void http::RequestReader::ReadContext::setRequestLine(const std::string &requestLine) {
    requestLine_ = requestLine;
}

void http::RequestReader::ReadContext::setHeaders(const RawHeaders &headers) {
    headers_ = headers;
}

void http::RequestReader::ReadContext::setBody(const std::string &body) {
    body_ = body;
}
