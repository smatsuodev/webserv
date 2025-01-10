#ifndef SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP
#define SRC_LIB_CORE_HANDLER_READ_REQUEST_HANDLER_HPP

#include "event/event_handler.hpp"
#include "http/request/reader/request_reader.hpp"
#include "../virtual_server_resolver.hpp"
#include <memory>

class ReadRequestHandler : public IEventHandler {
public:
    explicit ReadRequestHandler(const VirtualServerResolver &vsResolver);
    InvokeResult invoke(const Context &ctx);

private:
    std::auto_ptr<http::IConfigResolver> resolver_; // RequestReader に渡す参照先として必要
    http::RequestReader reqReader_;
};

class ConfigResolver : public http::IConfigResolver {
public:
    explicit ConfigResolver(const VirtualServerResolver &vsResolver);
    Option<config::ServerContext> resolve(const std::string &host) const;

private:
    VirtualServerResolver resolver_;
};

#endif
