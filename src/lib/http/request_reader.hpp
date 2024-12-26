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
    bufio::Reader &reader_;

    Option<std::string> rawRequestLine_;
    std::vector<std::string> headers_;
    bool headersParsed_;
    Option<size_t> contentLength_;
    char *bodyBuf_;
    size_t bodyBufSize_;
    Option<std::string> body_;

    Result<std::string, error::AppError> readLine() const;
};

#endif
