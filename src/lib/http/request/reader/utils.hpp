#ifndef SRC_LIB_HTTP_REQUEST_READER_UTILS_HPP
#define SRC_LIB_HTTP_REQUEST_READER_UTILS_HPP

#include "utils/io/read_buffer.hpp"
#include "utils/types/result.hpp"
#include "utils/types/error.hpp"

namespace http {
    class IConfigResolver;
}

namespace http {
    typedef std::vector<std::string> RawHeaders;

    typedef Result<Option<std::string>, error::AppError> GetLineResult;
    GetLineResult getLine(ReadBuffer &readBuf);

    Result<Option<std::string>, error::AppError> getHeaderValue(const RawHeaders &headers, const std::string &key);
}

#endif
