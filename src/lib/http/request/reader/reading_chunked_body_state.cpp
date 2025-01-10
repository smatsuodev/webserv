#include "reading_chunked_body_state.hpp"

namespace http {
    ReadingChunkedBodyState::ReadingChunkedBodyState(RequestReader &reader, const std::size_t clientMaxBodySize)
        : reader_(reader), clientMaxBodySize_(clientMaxBodySize) {}

    Result<RequestReader::IState::HandleStatus, error::AppError> ReadingChunkedBodyState::handle(ReadBuffer &readBuf) {
        return Err(error::kUnknown);
    }
}
