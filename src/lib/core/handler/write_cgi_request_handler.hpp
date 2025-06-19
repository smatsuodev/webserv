#ifndef SRC_LIB_CORE_HANDLER_WRITE_CGI_REQUEST_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_WRITE_CGI_REQUEST_HANDLER_HPP

#include "event/event_handler.hpp"
#include "cgi/request.hpp"

class WriteCgiRequestHandler : public IEventHandler {
public:
    WriteCgiRequestHandler(int cgiSocketFd, const cgi::Request &cgiRequest);
    ~WriteCgiRequestHandler();
    InvokeResult invoke(const Context &ctx);

private:
    int cgiSocketFd_;
    std::string requestData_;
    size_t totalBytesWritten_;
};

#endif
