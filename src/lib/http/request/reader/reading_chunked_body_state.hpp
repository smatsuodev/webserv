#ifndef SRC_LIB_HTTP_REQUEST_READER_READING_CHUNKED_BODY_STATE_HPP
#define SRC_LIB_HTTP_REQUEST_READER_READING_CHUNKED_BODY_STATE_HPP

#include "request_reader.hpp"

namespace http {
    class ReadingChunkedBodyState : public RequestReader::IState {
    public:
        ReadingChunkedBodyState(RequestReader &reader, std::size_t clientMaxBodySize);
        Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf);

    private:
        enum State { kReadingChunkSize, kReadingChunkData, kReadingTrailer, kDone };

        RequestReader &reader_;
        std::size_t clientMaxBodySize_;
        State state_;
        std::size_t chunkSize_;
        std::string body_;

        static Result<std::size_t, error::AppError> parseChunkSizeLine(const std::string &line);
    };
}

#endif
