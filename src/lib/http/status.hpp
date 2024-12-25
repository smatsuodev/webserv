#ifndef SRC_LIB_HTTP_STATUS_HPP
#define SRC_LIB_HTTP_STATUS_HPP

#include <string>

namespace http {
    enum HttpStatusCode {
        kStatusUnknown = 0,

        /* 1xx Informational */
        kStatusContinue = 100,
        kStatusSwitchingProtocols = 101,
        kStatusProcessing = 102,
        kStatusEarlyHints = 103,

        /* 2xx Success */
        kStatusOk = 200,
        kStatusCreated = 201,
        kStatusAccepted = 202,
        kStatusNonAuthoritativeInformation = 203,
        kStatusNoContent = 204,
        kStatusResetContent = 205,
        kStatusPartialContent = 206,
        kStatusMultiStatus = 207,
        kStatusAlreadyReported = 208,
        kStatusImUsed = 226,

        /* 3xx Redirection */
        kStatusMultipleChoices = 300,
        kStatusMovedPermanently = 301,
        kStatusFound = 302,
        kStatusSeeOther = 303,
        kStatusNotModified = 304,
        kStatusUseProxy = 305,
        kStatusTemporaryRedirect = 307,
        kStatusPermanentRedirect = 308,

        /* 4xx Client Error */
        kStatusBadRequest = 400,
        kStatusUnauthorized = 401,
        kStatusPaymentRequired = 402,
        kStatusForbidden = 403,
        kStatusNotFound = 404,
        kStatusMethodNotAllowed = 405,
        kStatusNotAcceptable = 406,
        kStatusProxyAuthenticationRequired = 407,
        kStatusRequestTimeout = 408,
        kStatusConflict = 409,
        kStatusGone = 410,
        kStatusLengthRequired = 411,
        kStatusPreconditionFailed = 412,
        kStatusPayloadTooLarge = 413,
        kStatusUriTooLong = 414,
        kStatusUnsupportedMediaType = 415,
        kStatusRangeNotSatisfiable = 416,
        kStatusExpectationFailed = 417,
        kStatusImATeapot = 418,
        kStatusMisdirectedRequest = 421,
        kStatusUnprocessableEntity = 422,
        kStatusLocked = 423,
        kStatusFailedDependency = 424,
        kStatusTooEarly = 425,
        kStatusUpgradeRequired = 426,
        kStatusPreconditionRequired = 428,
        kStatusTooManyRequests = 429,
        kStatusRequestHeaderFieldsTooLarge = 431,
        kStatusUnavailableForLegalReasons = 451,

        /* 5xx Server Error */
        kStatusInternalServerError = 500,
        kStatusNotImplemented = 501,
        kStatusBadGateway = 502,
        kStatusServiceUnavailable = 503,
        kStatusGatewayTimeout = 504,
        kStatusHttpVersionNotSupported = 505,
        kStatusVariantAlsoNegotiates = 506,
        kStatusInsufficientStorage = 507,
        kStatusLoopDetected = 508,
        kStatusNotExtended = 510,
        kStatusNetworkAuthenticationRequired = 511,
    };

    HttpStatusCode httpStatusCodeFromInt(int code);
    std::string getHttpStatusText(HttpStatusCode code);
}

#endif
