#include "event_handler.hpp"
#include "core/virtual_server_resolver.hpp"

IVirtualServerResolverFactory::~IVirtualServerResolverFactory() {}

Context::Context(
    const Event &event, const Option<Ref<Connection> > &conn, IVirtualServerResolverFactory &resolverFactory
)
    : event_(event), conn_(conn), resolverFactory_(resolverFactory) {}

Option<Ref<Connection> > Context::getConnection() const {
    return conn_;
}

const Event &Context::getEvent() const {
    return event_;
}

Option<VirtualServerResolver> Context::getResolver() const {
    if (conn_.isNone()) {
        return None;
    }
    return Some(resolverFactory_.create(conn_.unwrap()));
}

Context Context::withNewValues(const Event &event, const Option<Ref<Connection> > &conn) const {
    return Context(event, conn, resolverFactory_);
}

IEventHandler::~IEventHandler() {}
