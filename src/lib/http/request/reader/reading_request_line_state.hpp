#ifndef SRC_LIB_HTTP_REQUEST_READER_READING_REQUEST_LINE_STATE_HPP
#define SRC_LIB_HTTP_REQUEST_READER_READING_REQUEST_LINE_STATE_HPP

#include "./request_reader.hpp"

namespace http {
    class ReadingRequestLineState : public RequestReader::IState {
    public:
        explicit ReadingRequestLineState(RequestReader::ReadContext &ctx);
        Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf);

    private:
        RequestReader::ReadContext &ctx_;
    };
}

#endif
