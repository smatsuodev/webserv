#ifndef SRC_LIB_HTTP_REQUEST_READER_HPP
#define SRC_LIB_HTTP_REQUEST_READER_HPP

#include "../../http/request/request.hpp"
#include "../../utils/types/unit.hpp"
#include "event/event_handler.hpp"
#include <vector>

class RequestReader {
public:
    RequestReader();

    typedef Result<Option<http::Request>, error::AppError> ReadRequestResult;
    ReadRequestResult readRequest(const Context &ctx);

private:
    // 読み込みは途中で中断され得るので、どこまで進んだか記録する
    enum State {
        kReadingRequestLine,
        kReadingHeaders,
        kReadingBody,
        kDone,
    };

    typedef std::vector<std::string> RawHeaders;

    State state_;

    std::string requestLine_;
    RawHeaders headers_;
    Option<size_t> contentLength_;
    Option<std::string> body_;

    std::string bodyBuf_;

    typedef Result<Option<std::string>, error::AppError> GetLineResult;
    static GetLineResult getLine(ReadBuffer &readBuf);

    // private メンバに書き込むものと、戻り値で返すものが混在してる
    Result<Option<std::string>, error::AppError> getRequestLine(ReadBuffer &readBuf) const;
    Result<Option<types::Unit>, error::AppError> getHeaders(ReadBuffer &readBuf);
    static Result<Option<size_t>, error::AppError> getContentLength(const RawHeaders &);
    Result<Option<types::Unit>, error::AppError> getBody(ReadBuffer&readBuf,std::size_t);
};

#endif
