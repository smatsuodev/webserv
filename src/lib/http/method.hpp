#ifndef SRC_LIB_HTTP_METHOD_HPP
#define SRC_LIB_HTTP_METHOD_HPP

#include <string>

namespace http {
    enum HttpMethod {
        kMethodUnknown,
        kMethodGet,
        kMethodPost,
        kMethodPut,
        kMethodDelete,
        kMethodHead,
        kMethodOptions,
        kMethodTrace,
        kMethodConnect,
        kMethodPatch,
    };

    HttpMethod httpMethodFromString(const std::string &method);
    std::string httpMethodToString(HttpMethod method);
}

#endif
