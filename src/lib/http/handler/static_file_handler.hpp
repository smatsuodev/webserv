#ifndef SRC_LIB_HTTP_STATIC_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_STATIC_FILE_HANDLER_HPP

#include "handler.hpp"

namespace http {
    class StaticFileHandler : public IHandler {
    public:
        virtual Response serve(const Request &req);
    };
} // http


#endif
