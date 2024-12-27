#ifndef SRC_LIB_HTTP_REQUEST_READER_HPP
#define SRC_LIB_HTTP_REQUEST_READER_HPP

#include "request.hpp"
#include "transport/connection.hpp"
#include <vector>

class RequestReader {
public:
    explicit RequestReader(bufio::Reader &reader);
    ~RequestReader();

    typedef Result<http::Request, error::AppError> ReadRequestResult;
    ReadRequestResult readRequest();

private:
    // 読み込みは途中で中断され得るので、どこまで進んだか記録する
    enum State {
        kReadingRequestLine,
        kReadingHeaders,
        kReadingBody,
        kDone,
    };

    bufio::Reader &reader_;
    State state_;

    std::string rawRequestLine_;
    std::vector<std::string> headers_;
    Option<size_t> contentLength_;
    char *bodyBuf_;
    size_t bodyBufSize_;
    Option<std::string> body_;

    Result<std::string, error::AppError> readLine() const;

    Result<void, error::AppError> readRequestLine();
    Result<void, error::AppError> readHeaders();
    Result<void, error::AppError> readBody();
};

#endif
