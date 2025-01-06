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

    std::string httpMethodToString(const HttpMethod method) {
        switch (method) {
            case kMethodGet:
                return "GET";
            case kMethodPost:
                return "POST";
            case kMethodPut:
                return "PUT";
            case kMethodDelete:
                return "DELETE";
            case kMethodHead:
                return "HEAD";
            case kMethodOptions:
                return "OPTIONS";
            case kMethodTrace:
                return "TRACE";
            case kMethodConnect:
                return "CONNECT";
            case kMethodPatch:
                return "PATCH";
            default:
                return "UNKNOWN";
        }
    }

}
