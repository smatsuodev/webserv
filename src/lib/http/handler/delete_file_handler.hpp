#ifndef SRC_LIB_HTTP_DELETE_FILE_HANDLER_HPP
#define SRC_LIB_HTTP_DELETE_FILE_HANDLER_HPP
#include "handler.hpp"
#include "delete_file_handler.hpp"

namespace http {
    class DeleteFileHandler : public IHandler {
    public:
        Response serve(const Request &req);
    };
} // http

#endif
