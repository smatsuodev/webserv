#include "event_handler.hpp"

Context::Context(Connection &conn, const Event &event) : conn_(conn), event_(event) {}

const Connection &Context::getConnection() const {
    return conn_;
}

const Event &Context::getEvent() const {
    return event_;
}
