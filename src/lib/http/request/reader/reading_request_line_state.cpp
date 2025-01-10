#include "reading_request_line_state.hpp"

#include "reading_headers_state.hpp"
#include "utils.hpp"
#include "utils/types/try.hpp"

http::ReadingRequestLineState::ReadingRequestLineState(RequestReader &reader) : reader_(reader) {}

Result<http::RequestReader::IState::HandleStatus, error::AppError>
http::ReadingRequestLineState::handle(ReadBuffer &readBuf) {
    const Option<std::string> reqLine = TRY(getLine(readBuf));
    if (reqLine.isNone()) {
        return Ok(HandleStatus::kSuspend);
    }

    reader_.setRequestLine(reqLine.unwrap());
    reader_.changeState(new ReadingHeadersState(reader_));

    return Ok(HandleStatus::kDone);
}
