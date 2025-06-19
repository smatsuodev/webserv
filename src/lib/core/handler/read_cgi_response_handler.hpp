#ifndef SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP

#include "event/event_handler.hpp"
#include "cgi/response.hpp"
#include <string>

class ReadCgiResponseHandler : public IEventHandler {
public:
    ReadCgiResponseHandler(int cgiSocketFd, pid_t cgiPid);
    ~ReadCgiResponseHandler();
    InvokeResult invoke(const Context &ctx);

private:
    int cgiSocketFd_;
    pid_t cgiPid_;
    std::string responseBuffer_;
    bool isResponseComplete_;

    bool checkCgiProcessStatus();
};

#endif
