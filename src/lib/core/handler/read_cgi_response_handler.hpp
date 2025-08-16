#ifndef SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP

#include "event/event_handler.hpp"
#include "cgi/response.hpp"
#include "http/response/response.hpp"

class ReadCgiResponseHandler : public IEventHandler {
public:
    ReadCgiResponseHandler(int clientFd, pid_t childPid);
    ~ReadCgiResponseHandler();

    InvokeResult invoke(const Context &ctx);
    ErrorHandleResult onErrorEvent(const Context &ctx, const Event &event);

private:
    std::string responseBuffer_;
    int clientFd_;
    pid_t childPid_;

    static std::vector<IAction *> makeNextActions(Connection &conn, int clientFd, const http::Response &httpResponse);
    static http::HttpStatusCode determineStatusCode(const cgi::Response &response);
    static Result<cgi::Response, error::AppError> createCgiResponseFromBuffer(const std::string &buf);
    static http::Response toHttpResponse(const cgi::Response &response);
};

#endif
