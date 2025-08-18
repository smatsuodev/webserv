#include "reading_request_line_state.hpp"
#include "reading_headers_state.hpp"
#include "utils.hpp"
#include "utils/types/try.hpp"

http::ReadingRequestLineState::ReadingRequestLineState(RequestReader::ReadContext &ctx) : ctx_(ctx) {}

Result<http::RequestReader::IState::HandleStatus, error::AppError>
http::ReadingRequestLineState::handle(ReadBuffer &readBuf) {
    const Option<std::string> reqLine = TRY(getLine(readBuf));
    if (reqLine.isNone()) {
        return Ok(kSuspend);
    }

    ctx_.setRequestLine(reqLine.unwrap());
    ctx_.changeState(new ReadingHeadersState(ctx_));

    return Ok(kHandleDone);
}
