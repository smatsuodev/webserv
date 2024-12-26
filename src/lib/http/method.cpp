#include "method.hpp"

namespace http {
    HttpMethod httpMethodFromString(const std::string &method) {
        if (method == "GET") {
            return kMethodGet;
        }
        if (method == "POST") {
            return kMethodPost;
        }
        if (method == "PUT") {
            return kMethodPut;
        }
        if (method == "DELETE") {
            return kMethodDelete;
        }
        if (method == "HEAD") {
            return kMethodHead;
        }
        if (method == "OPTIONS") {
            return kMethodOptions;
        }
        if (method == "TRACE") {
            return kMethodTrace;
        }
        if (method == "CONNECT") {
            return kMethodConnect;
        }
        if (method == "PATCH") {
            return kMethodPatch;
        }
        return kMethodUnknown;
    }
}
