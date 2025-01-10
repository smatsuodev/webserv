#ifndef SRC_LIB_HTTP_REQUEST_READER_READING_BODY_STATE_HPP
#define SRC_LIB_HTTP_REQUEST_READER_READING_BODY_STATE_HPP

#include "./request_reader.hpp"

namespace http {
    class ReadingBodyState : public RequestReader::IState {
    public:
        ReadingBodyState(RequestReader::ReadContext &ctx, std::size_t contentLength);
        Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf);

    private:
        RequestReader::ReadContext &ctx_;
        std::size_t contentLength_;
        std::string body_;
    };
}

#endif
