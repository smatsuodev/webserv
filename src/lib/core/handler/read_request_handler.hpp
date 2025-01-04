#ifndef SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP

#include "event/event_handler.hpp"
#include "request_reader.hpp"

class ReadRequestHandler : public IEventHandler {
public:
    // NOTE: ReadBuffer に直接依存するべきか?
    explicit ReadRequestHandler(ReadBuffer &readBuf);
    InvokeResult invoke(const Context &ctx);

private:
    RequestReader reqReader_;
};

#endif
