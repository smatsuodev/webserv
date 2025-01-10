#ifndef SRC_LIB_HTTP_REQUEST_READER_READING_HEADERS_STATE_HPP
#define SRC_LIB_HTTP_REQUEST_READER_READING_HEADERS_STATE_HPP

#include "./request_reader.hpp"

namespace http {
    class ReadingHeadersState : public RequestReader::IState {
    public:
        explicit ReadingHeadersState(RequestReader &reader);
        Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf);

    private:
        RequestReader &reader_;
        RawHeaders headers_;

        Result<std::size_t, error::AppError> getClientMaxBodySize(const RawHeaders &headers) const;
        static Result<Option<std::size_t>, error::AppError> getContentLength(const RawHeaders &headers);
    };
}

#endif
