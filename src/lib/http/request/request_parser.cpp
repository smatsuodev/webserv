#include "request_parser.hpp"
#include "utils/logger.hpp"
#include "utils/types/try.hpp"

namespace http {
    RequestParser::ParseResult RequestParser::parseRequest(const std::string &requestLine,
                                                           const std::vector<std::string> &headers,
                                                           const std::string &body) {
        // TODO: parse query
        const RequestLine parsedRequestLine = TRY(parseRequestLine(requestLine));

        std::map<std::string, std::string> parsedHeaders;
        for (std::size_t i = 0; i < headers.size(); i++) {
            parsedHeaders.insert(TRY(parseHeaderFieldLine(headers[i])));
        }

        return Ok(Request(parsedRequestLine.first.first, // method
                          parsedRequestLine.first.second, // request-target
                          parsedRequestLine.second, // HTTP-version
                          parsedHeaders, body));
    }

    // field-line = field-name ":" OWS field-value OWS
    RequestParser::ParseHeaderFieldLineResult RequestParser::parseHeaderFieldLine(const std::string &line) {
        // : の前後で区切る
        const std::size_t colonPos = line.find(':');
        if (colonPos == std::string::npos) {
            LOG_DEBUG("parseHeaderFieldLine: missing colon");
            return Err(error::kParseUnknown);
        }
        const std::string rawFieldName = line.substr(0, colonPos);
        const std::string rawFieldValue = line.substr(colonPos + 1);

        // field-name の検証
        if (!isValidFieldName(rawFieldName)) {
            LOG_DEBUG("parseHeaderFieldLine: invalid field-name");
            return Err(error::kParseUnknown);
        }

        // field-value の検証
        // OWS = *( SP / HTAB ) で trim する
        /* (RFC 9110)
         * A field value does not include leading or trailing whitespace.
         * When a specific version of HTTP allows such whitespace to appear in a message,
         * a field parsing implementation MUST exclude such whitespace
         * prior to evaluating the field value.
         */
        const std::string::size_type begin = rawFieldValue.find_first_not_of(" \t");
        const std::string::size_type end = rawFieldValue.find_last_not_of(" \t");
        if (begin == std::string::npos || end == std::string::npos) {
            LOG_DEBUG("parseHeaderFieldLine: empty field-value");
            return Err(error::kParseUnknown);
        }
        const std::string trimmedRawFieldValue = rawFieldValue.substr(begin, end - begin + 1);
        if (!isValidFieldValue(trimmedRawFieldValue)) {
            LOG_DEBUG("parseHeaderFieldLine: invalid field-value");
            return Err(error::kParseUnknown);
        }

        return Ok(std::make_pair(rawFieldName, trimmedRawFieldValue));
    }

    // request-line = method SP request-target SP HTTP-version
    // NOTE: サーバーは SP 以外にも HTAB, VT, FF, CR を区切りとしてもよい (MAY)
    RequestParser::ParseRequestLineResult RequestParser::parseRequestLine(const std::string &line) {
        // 最初と最後のスペースを探し, 3分割する
        const std::string::size_type firstSpacePos = line.find(' ');
        const std::string::size_type lastSpacePos = line.find_last_of(' ');
        if (firstSpacePos == std::string::npos || lastSpacePos == std::string::npos || firstSpacePos == lastSpacePos) {
            LOG_DEBUG("parseRequestLine: invalid format");
            return Err(error::kParseUnknown);
        }
        const std::string rawMethod = line.substr(0, firstSpacePos);
        const std::string rawRequestTarget = line.substr(firstSpacePos + 1, lastSpacePos - firstSpacePos - 1);
        const std::string rawHttpVersion = line.substr(lastSpacePos + 1);

        // TODO: request-target の validation
        const HttpMethod method = httpMethodFromString(rawMethod);
        if (method == kMethodUnknown) {
            LOG_DEBUG("parseRequestLine: unknown method");
            return Err(error::kParseUnknown);
        }
        if (!isValidHttpVersion(rawHttpVersion)) {
            LOG_DEBUG("parseRequestLine: invalid http version");
            return Err(error::kParseUnknown);
        }

        return Ok(std::make_pair(std::make_pair(method, rawRequestTarget), rawHttpVersion));
    }

    /*
     * field-name = token
     * token = 1*tchar
     * tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*"
     *       / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
     *       / DIGIT / ALPHA ; any VCHAR, except delimiters
     */
    bool RequestParser::isValidFieldName(const std::string &fieldName) {
        if (fieldName.empty()) {
            return false;
        }

        for (size_t i = 0; i < fieldName.size(); i++) {
            const unsigned char c = fieldName[i];
            if (!(std::isalnum(c) || c == '!' || c == '#' || c == '$' || c == '%' || c == '&' || c == '\'' ||
                  c == '*' || c == '+' || c == '-' || c == '.' || c == '^' || c == '_' || c == '`' || c == '|' ||
                  c == '~')) {
                return false;
            }
        }
        return true;
    }

    /*
     * field-value = *field-content
     * field-content = field-vchar [ 1*( SP / HTAB / field-vchar ) field-vchar) ]
     * field-vchar = VCHAR / obs-text
     * obs-text = %x80-FF
     */
    // NOTE: trim 済みであることを仮定すれば, field-value = *field-vchar
    bool RequestParser::isValidFieldValue(const std::string &fieldValue) {
        if (fieldValue.empty()) {
            return true;
        }

        // 先頭・末尾が SP / HTAB でないことを確認
        if (fieldValue[0] == ' ' || fieldValue[0] == '\t' || fieldValue[fieldValue.size() - 1] == ' ' ||
            fieldValue[fieldValue.size() - 1] == '\t') {
            return false;
        }

        // 最後に *( SP / HTAB / field-vchar ) であることを確認すればよい
        for (size_t i = 0; i < fieldValue.size(); i++) {
            const unsigned char c = fieldValue[i];
            // TODO: std::isprint で適切か
            // c <= 0xff は常に成り立つので省略
            if (!(std::isprint(c) || c >= 0x80)) {
                return false;
            }
        }

        return true;
    }

    bool RequestParser::isValidHttpVersion(const std::string &httpVersion) {
        // HTTP/1.1 の形式, 8文字
        if (!(httpVersion.size() == 8 && utils::startsWith(httpVersion, "HTTP/"))) {
            return false;
        }
        // バージョンをチェック
        const char major = httpVersion[5];
        const char minor = httpVersion[7];
        return httpVersion[6] == '.' && std::isdigit(major) && std::isdigit(minor);
    }
}
