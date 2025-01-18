#ifndef SRC_LIB_HTTP_REQUEST_READER_READING_HEADERS_STATE_HPP
#define SRC_LIB_HTTP_REQUEST_READER_READING_HEADERS_STATE_HPP

#include "./request_reader.hpp"

namespace http {
    class ReadingHeadersState : public RequestReader::IState {
    public:
        explicit ReadingHeadersState(RequestReader::ReadContext &ctx);
        Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf);

    private:
        RequestReader::ReadContext &ctx_;
        RawHeaders headers_;

        Result<IState *, error::AppError> nextState() const;

        Result<std::size_t, error::AppError> getClientMaxBodySize(const RawHeaders &headers) const;
        static Result<Option<std::size_t>, error::AppError> getContentLength(const RawHeaders &headers);
        static Result<bool, error::AppError> isChunked(const RawHeaders &headers);
    };
}

#endif
