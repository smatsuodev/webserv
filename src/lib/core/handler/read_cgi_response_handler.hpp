#ifndef SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#include "event/event_handler.hpp"
#include "cgi/response.hpp"
#include "utils/types/option.hpp"

class ReadCgiResponseHandler : public IEventHandler {
public:
    ReadCgiResponseHandler(int clientFd, pid_t childPid);
    ~ReadCgiResponseHandler();

    InvokeResult invoke(const Context &ctx);
    ErrorHandleResult onErrorEvent(const Context &ctx, const Event &event) override;

private:
    std::string responseBuffer_;
    Option<cgi::Response> cgiResponse_;
    int clientFd_;
    pid_t childPid_;
};

#endif
