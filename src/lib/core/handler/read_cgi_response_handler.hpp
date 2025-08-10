<<<<<<< ours
#ifndef SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#include "event/event_handler.hpp"

class ReadCgiResponseHandler : public IEventHandler {
public:
    ReadCgiResponseHandler();
    ~ReadCgiResponseHandler();

    InvokeResult invoke(const Context &ctx);

private:
};

#endif
|||||||
=======
#ifndef SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_CGI_RESPONSE_HANDLER_HPP

class ReadCgiResponseHandler {
public:
    ReadCgiResponseHandler();
    ~ReadCgiResponseHandler();

private:
};

#endif
>>>>>>> theirs
