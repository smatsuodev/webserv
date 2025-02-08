#ifndef SRC_LIB_CGI_RESPONSE_HPP
#define SRC_LIB_CGI_RESPONSE_HPP
#include "utils/types/error.hpp"
#include "utils/types/option.hpp"


#include <map>

namespace cgi {
    typedef std::map<std::string, std::string> Headers;

    /**
     * generic-response = 1*header-field NL [ response-body ]
     *
     * NOTE: document-response だけ対応する
     * CGI-Response = document-response | local-redir-response | client-redir-response | client-redirdoc-response
     * document-response = Content-Type [ Status ] *other-field NL response-body
     * Status = "Status:" status-code SP reason-phrase NL
     * status-code = "200" | "302" | "400" | "501" | extension-code
     * other-field = protocol-field | extension-field
     */
    class Response {
    public:
        // document-response は Content-Type が必須。ない場合はエラーが返る。
        static Result<Response, error::AppError> create(const Headers &headers, const Option<std::string> &body = None);

        const Headers &getHeaders() const;
        const Option<std::string> &getBody() const;

    private:
        Headers headers_;
        Option<std::string> body_;

        Response(const Headers &headers, const Option<std::string> &body);
    };
}

#endif
