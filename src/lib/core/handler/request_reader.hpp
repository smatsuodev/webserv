#ifndef SRC_LIB_HTTP_REQUEST_READER_HPP
#define SRC_LIB_HTTP_REQUEST_READER_HPP

#include "../../http/request/request.hpp"
#include "../../utils/types/unit.hpp"
#include "event/event_handler.hpp"
#include "config/config.hpp"
#include "../virtual_server_resolver.hpp"
#include <vector>

class RequestReader {
public:
    /**
     * config::Resolver は (localAddress, Host) -> ServerContext
     * RequestReader をテストしやすくするための IF
     */
    class IConfigResolver {
    public:
        virtual ~IConfigResolver();
        virtual Option<config::ServerContext> resolve(const std::string &host) const = 0;
    };
    class ConfigResolver : public IConfigResolver {
    public:
        explicit ConfigResolver(const VirtualServerResolver &vsResolver);
        Option<config::ServerContext> resolve(const std::string &host) const;

    private:
        VirtualServerResolver resolver_;
    };

    explicit RequestReader(IConfigResolver &resolver);

    typedef Result<Option<http::Request>, error::AppError> ReadRequestResult;
    ReadRequestResult readRequest(ReadBuffer &readBuf);

private:
    // 読み込みは途中で中断され得るので、どこまで進んだか記録する
    enum State {
        kReadingRequestLine,
        kReadingHeaders,
        kReadingBody,
        kDone,
    };

    typedef std::vector<std::string> RawHeaders;

    IConfigResolver &resolver_;
    State state_;

    std::string requestLine_;
    RawHeaders headers_;
    Option<std::size_t> contentLength_;
    std::size_t clientMaxBodySize_; // Host ヘッダー読み込み後に解決される
    Option<std::string> body_;

    std::string bodyBuf_;

    typedef Result<Option<std::string>, error::AppError> GetLineResult;
    static GetLineResult getLine(ReadBuffer &readBuf);

    // private メンバに書き込むものと、戻り値で返すものが混在してる
    Result<Option<std::string>, error::AppError> getRequestLine(ReadBuffer &readBuf) const;
    Result<Option<types::Unit>, error::AppError> getHeaders(ReadBuffer &readBuf);
    static Result<Option<size_t>, error::AppError> getContentLength(const RawHeaders &);
    Result<std::size_t, error::AppError> resolveClientMaxBodySize(const RawHeaders &) const;
    Result<Option<types::Unit>, error::AppError> getBody(ReadBuffer &readBuf, std::size_t);
};

#endif
