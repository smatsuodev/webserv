#include "event_handler.hpp"

Context::Context(const Option<Ref<Connection> > &conn, const Event &event, const config::Resolver &resolver)
    : conn_(conn), event_(event), resolver_(resolver) {}

Option<Ref<Connection> > Context::getConnection() const {
    return conn_;
}

const Event &Context::getEvent() const {
    return event_;
}

const config::Resolver &Context::getResolver() const {
    return resolver_;
}

IEventHandler::~IEventHandler() {}
