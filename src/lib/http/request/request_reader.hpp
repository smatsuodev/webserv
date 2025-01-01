#ifndef SRC_LIB_HTTP_REQUEST_READER_HPP
#define SRC_LIB_HTTP_REQUEST_READER_HPP

#include "request.hpp"
#include "transport/connection.hpp"
#include <vector>

class RequestReader {
public:
    // NOTE: ReadBuffer に直接依存するべきか?
    explicit RequestReader(ReadBuffer &readBuf);

    typedef Result<Option<http::Request>, error::AppError> ReadRequestResult;
    ReadRequestResult readRequest();

private:
    // 読み込みは途中で中断され得るので、どこまで進んだか記録する
    enum State {
        kReadingRequestLine,
        kReadingHeaders,
        kReadingBody,
        kDone,
    };

    typedef std::vector<std::string> RawHeaders;

    ReadBuffer &readBuf_;
    State state_;

    std::string requestLine_;
    RawHeaders headers_;
    Option<size_t> contentLength_;
    Option<std::string> body_;

    typedef Result<Option<std::string>, error::AppError> GetLineResult;
    static GetLineResult getLine(ReadBuffer &);


    static Result<Option<std::string>, error::AppError> getRequestLine(ReadBuffer &);
    static Result<Option<RawHeaders>, error::AppError> getHeaders(ReadBuffer &);
    static Result<Option<size_t>, error::AppError> getContentLength(const RawHeaders &);
    static Result<Option<std::string>, error::AppError> getBody(ReadBuffer &, std::size_t);
};

#endif
