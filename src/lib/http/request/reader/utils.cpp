#include "utils.hpp"
#include "config/config.hpp"
#include "http/request/request_parser.hpp"
#include "utils/string.hpp"
#include "utils/types/try.hpp"
#include "./request_reader.hpp"

http::GetLineResult http::getLine(ReadBuffer &readBuf) {
    const Option<std::string> maybeLine = readBuf.consumeUntil("\r\n");
    if (maybeLine.isNone()) {
        return Ok(None);
    }

    std::string line = maybeLine.unwrap();
    if (!utils::endsWith(line, "\r\n")) {
        return Err(error::kParseUnknown);
    }

    line.erase(line.size() - 2);
    return Ok(Some(line));
}

Result<Option<std::string>, error::AppError> http::getHeaderValue(const RawHeaders &headers, const std::string &key) {
    for (RawHeaders::const_iterator it = headers.begin(); it != headers.end(); ++it) {
        const RequestParser::HeaderField &field = TRY(RequestParser::parseHeaderFieldLine(*it));
        if (field.first == key) {
            return Ok(Some(field.second));
        }
    }
    return Ok(None);
}
