#ifndef SRC_LIB_HTTP_REQUEST_READER_READING_CHUNKED_BODY_STATE_HPP
#define SRC_LIB_HTTP_REQUEST_READER_READING_CHUNKED_BODY_STATE_HPP

#include "request_reader.hpp"

namespace http {
    class ReadingChunkedBodyState : public RequestReader::IState {
    public:
        ReadingChunkedBodyState(RequestReader &reader, std::size_t clientMaxBodySize);
        Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf);

    private:
        RequestReader &reader_;
        std::size_t clientMaxBodySize_;
    };
}

#endif
