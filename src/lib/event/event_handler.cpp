#include "event_handler.hpp"

Context::Context(const Option<Ref<Connection> > &conn, const Event &event) : conn_(conn), event_(event) {}

Option<Ref<Connection> > Context::getConnection() const {
    return conn_;
}

const Event &Context::getEvent() const {
    return event_;
}

IEventHandler::~IEventHandler() {}
