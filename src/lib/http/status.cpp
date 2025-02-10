#include "status.hpp"

namespace http {
    Option<HttpStatusCode> httpStatusCodeFromInt(int code) {
        switch (code) {
            case kStatusContinue:
            case kStatusSwitchingProtocols:
            case kStatusProcessing:
            case kStatusEarlyHints:
            case kStatusOk:
            case kStatusCreated:
            case kStatusAccepted:
            case kStatusNonAuthoritativeInformation:
            case kStatusNoContent:
            case kStatusResetContent:
            case kStatusPartialContent:
            case kStatusMultiStatus:
            case kStatusAlreadyReported:
            case kStatusImUsed:
            case kStatusMultipleChoices:
            case kStatusMovedPermanently:
            case kStatusFound:
            case kStatusSeeOther:
            case kStatusNotModified:
            case kStatusUseProxy:
            case kStatusTemporaryRedirect:
            case kStatusPermanentRedirect:
            case kStatusBadRequest:
            case kStatusUnauthorized:
            case kStatusPaymentRequired:
            case kStatusForbidden:
            case kStatusNotFound:
            case kStatusMethodNotAllowed:
            case kStatusNotAcceptable:
            case kStatusProxyAuthenticationRequired:
            case kStatusRequestTimeout:
            case kStatusConflict:
            case kStatusGone:
            case kStatusLengthRequired:
            case kStatusPreconditionFailed:
            case kStatusPayloadTooLarge:
            case kStatusUriTooLong:
            case kStatusUnsupportedMediaType:
            case kStatusRangeNotSatisfiable:
            case kStatusExpectationFailed:
            case kStatusImATeapot:
            case kStatusMisdirectedRequest:
            case kStatusUnprocessableEntity:
            case kStatusLocked:
            case kStatusFailedDependency:
            case kStatusTooEarly:
            case kStatusUpgradeRequired:
            case kStatusPreconditionRequired:
            case kStatusTooManyRequests:
            case kStatusRequestHeaderFieldsTooLarge:
            case kStatusUnavailableForLegalReasons:
            case kStatusInternalServerError:
            case kStatusNotImplemented:
            case kStatusBadGateway:
            case kStatusServiceUnavailable:
            case kStatusGatewayTimeout:
            case kStatusHttpVersionNotSupported:
            case kStatusVariantAlsoNegotiates:
            case kStatusInsufficientStorage:
            case kStatusLoopDetected:
            case kStatusNotExtended:
            case kStatusNetworkAuthenticationRequired:
                return Some(static_cast<HttpStatusCode>(code));
            default:
                return None;
        }
    }

    std::string getHttpStatusText(const HttpStatusCode code) {
        switch (code) {
            case kStatusContinue:
                return "Continue";
            case kStatusSwitchingProtocols:
                return "Switching Protocols";
            case kStatusProcessing:
                return "Processing";
            case kStatusEarlyHints:
                return "Early Hints";
            case kStatusOk:
                return "OK";
            case kStatusCreated:
                return "Created";
            case kStatusAccepted:
                return "Accepted";
            case kStatusNonAuthoritativeInformation:
                return "Non-Authoritative Information";
            case kStatusNoContent:
                return "No Content";
            case kStatusResetContent:
                return "Reset Content";
            case kStatusPartialContent:
                return "Partial Content";
            case kStatusMultiStatus:
                return "Multi-Status";
            case kStatusAlreadyReported:
                return "Already Reported";
            case kStatusImUsed:
                return "IM Used";
            case kStatusMultipleChoices:
                return "Multiple Choices";
            case kStatusMovedPermanently:
                return "Moved Permanently";
            case kStatusFound:
                return "Found";
            case kStatusSeeOther:
                return "See Other";
            case kStatusNotModified:
                return "Not Modified";
            case kStatusUseProxy:
                return "Use Proxy";
            case kStatusTemporaryRedirect:
                return "Temporary Redirect";
            case kStatusPermanentRedirect:
                return "Permanent Redirect";
            case kStatusBadRequest:
                return "Bad Request";
            case kStatusUnauthorized:
                return "Unauthorized";
            case kStatusPaymentRequired:
                return "Payment Required";
            case kStatusForbidden:
                return "Forbidden";
            case kStatusNotFound:
                return "Not Found";
            case kStatusMethodNotAllowed:
                return "Method Not Allowed";
            case kStatusNotAcceptable:
                return "Not Acceptable";
            case kStatusProxyAuthenticationRequired:
                return "Proxy Authentication Required";
            case kStatusRequestTimeout:
                return "Request Timeout";
            case kStatusConflict:
                return "Conflict";
            case kStatusGone:
                return "Gone";
            case kStatusLengthRequired:
                return "Length Required";
            case kStatusPreconditionFailed:
                return "Precondition Failed";
            case kStatusPayloadTooLarge:
                return "Payload Too Large";
            case kStatusUriTooLong:
                return "URI Too Long";
            case kStatusUnsupportedMediaType:
                return "Unsupported Media Type";
            case kStatusRangeNotSatisfiable:
                return "Range Not Satisfiable";
            case kStatusExpectationFailed:
                return "Expectation Failed";
            case kStatusImATeapot:
                return "I'm a teapot";
            case kStatusMisdirectedRequest:
                return "Misdirected Request";
            case kStatusUnprocessableEntity:
                return "Unprocessable Entity";
            case kStatusLocked:
                return "Locked";
            case kStatusFailedDependency:
                return "Failed Dependency";
            case kStatusTooEarly:
                return "Too Early";
            case kStatusUpgradeRequired:
                return "Upgrade Required";
            case kStatusPreconditionRequired:
                return "Precondition Required";
            case kStatusTooManyRequests:
                return "Too Many Requests";
            case kStatusRequestHeaderFieldsTooLarge:
                return "Request Header Fields Too Large";
            case kStatusUnavailableForLegalReasons:
                return "Unavailable For Legal Reasons";
            case kStatusInternalServerError:
                return "Internal Server Error";
            case kStatusNotImplemented:
                return "Not Implemented";
            case kStatusBadGateway:
                return "Bad Gateway";
            case kStatusServiceUnavailable:
                return "Service Unavailable";
            case kStatusGatewayTimeout:
                return "Gateway Timeout";
            case kStatusHttpVersionNotSupported:
                return "HTTP Version Not Supported";
            case kStatusVariantAlsoNegotiates:
                return "Variant Also Negotiates";
            case kStatusInsufficientStorage:
                return "Insufficient Storage";
            case kStatusLoopDetected:
                return "Loop Detected";
            case kStatusNotExtended:
                return "Not Extended";
            case kStatusNetworkAuthenticationRequired:
                return "Network Authentication Required";
            default:
                return "";
        }
    }
}
