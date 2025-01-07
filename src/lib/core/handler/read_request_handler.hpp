#ifndef SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP

#include "event/event_handler.hpp"
#include "request_reader.hpp"
#include <memory>

class ReadRequestHandler : public IEventHandler {
public:
    explicit ReadRequestHandler(const VirtualServerResolver &vsResolver);
    InvokeResult invoke(const Context &ctx);

private:
    std::auto_ptr<RequestReader::IConfigResolver> resolver_; // RequestReader に渡す参照先として必要
    RequestReader reqReader_;
};

#endif
