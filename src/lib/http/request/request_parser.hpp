#ifndef SRC_LIB_HTTP_REQUEST_PARSER_HPP
#define SRC_LIB_HTTP_REQUEST_PARSER_HPP

#include "request.hpp"
#include "utils/types/error.hpp"
#include "utils/types/result.hpp"
#include <vector>

namespace http {
    class RequestParser {
    public:
        typedef std::pair<std::string, std::string> HeaderField;

        typedef Result<Request, error::AppError> ParseResult;
        static ParseResult
        parseRequest(const std::string &requestLine, const std::vector<std::string> &headers, const std::string &body);

        typedef Result<HeaderField, error::AppError> ParseHeaderFieldLineResult;
        static ParseHeaderFieldLineResult parseHeaderFieldLine(const std::string &line);

    private:
        /**
         * TODO: 分かりにくいのでクラスにする? (使用箇所は限定的)
         * tuple は C++11 から
         */
        // method, request-target, HTTP-version
        typedef std::pair<std::pair<HttpMethod, std::string>, std::string> RequestLine;

        typedef Result<RequestLine, error::AppError> ParseRequestLineResult;
        static ParseRequestLineResult parseRequestLine(const std::string &line);
        static bool isValidFieldName(const std::string &fieldName);
        static bool isValidFieldValue(const std::string &fieldValue);
        static bool isValidHttpVersion(const std::string &httpVersion);
    };
}

#endif
