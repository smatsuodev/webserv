#ifndef SRC_LIB_HTTP_REQUEST_READER_REQUEST_READER_HPP
#define SRC_LIB_HTTP_REQUEST_READER_REQUEST_READER_HPP

#include "http/request/request.hpp"
#include "utils/io/read_buffer.hpp"
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"
#include "./utils.hpp"

namespace config {
    class ServerContext;
}

namespace http {
    /**
     * header を読み込むまで、どの client_max_body_size を使うかわからない
     * この IF を使って client_max_body_size (を含む config::ServerContext) を解決する
     */
    class IConfigResolver {
    public:
        virtual ~IConfigResolver();
        virtual Option<config::ServerContext> resolve(const std::string &host) const = 0;
    };

    class RequestReader {
    public:
        class IState {
        public:
            enum HandleStatus { kSuspend, kDone };
            virtual ~IState();
            virtual Result<HandleStatus, error::AppError> handle(ReadBuffer &readBuf) = 0;
        };

        explicit RequestReader(IConfigResolver &resolver);
        ~RequestReader();

        typedef Result<Option<Request>, error::AppError> ReadRequestResult;
        ReadRequestResult readRequest(ReadBuffer &readBuf) const;

        void changeState(IState *state);

        IConfigResolver &getConfigResolver() const;
        void setRequestLine(const std::string &requestLine);
        void setHeaders(const RawHeaders &headers);
        void setBody(const std::string &body);

    private:
        // std::auto_ptr にしたいが、テストで使う C++ のバージョンでは削除されている
        IState *state_;
        IConfigResolver &resolver_;
        std::string requestLine_;
        RawHeaders headers_;
        std::string body_;
    };
}

#endif
