#include "reading_body_state.hpp"
#include "core/virtual_server.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

http::ReadingBodyState::ReadingBodyState(RequestReader &reader, const std::size_t contentLength)
    : reader_(reader), contentLength_(contentLength) {}

Result<http::RequestReader::IState::HandleStatus, error::AppError> http::ReadingBodyState::handle(ReadBuffer &readBuf) {
    while (body_.size() < contentLength_) {
        const std::size_t want = contentLength_ - body_.size();
        const std::string chunk = readBuf.consume(want);
        if (chunk.empty()) {
            return Ok(HandleStatus::kSuspend);
        }
        body_ += chunk;
    }

    reader_.setBody(body_);
    reader_.changeState(NULL);

    return Ok(HandleStatus::kDone);
}
