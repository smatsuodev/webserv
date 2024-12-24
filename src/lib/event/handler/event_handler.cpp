#include "event_handler.hpp"
#include "utils/types/try.hpp"

Context::Context(Server &server, const Option<Connection &> &conn, const Event &event)
    : server_(server), conn_(conn), event_(event) {}

Server &Context::getServer() const {
    return server_;
}

Option<Connection &> Context::getConnection() const {
    return conn_;
}

const Event &Context::getEvent() const {
    return event_;
}
