#ifndef SRC_LIB_CORE_ACTION_HPP
#define SRC_LIB_CORE_ACTION_HPP

#include "event/event_handler.hpp"

class AddConnectionAction : public IAction {
public:
    explicit AddConnectionAction(Connection *conn);
    void execute(ServerState &state);

private:
    Connection *conn_;
};

class RemoveConnectionAction : public IAction {
public:
    explicit RemoveConnectionAction(Connection &conn);
    void execute(ServerState &state);

private:
    Connection &conn_;
    bool executed_;
};

class RegisterEventHandlerAction : public IAction {
public:
    RegisterEventHandlerAction(Connection &conn, IEventHandler *handler);
    void execute(ServerState &state);

private:
    Connection &conn_;
    IEventHandler *handler_;
    bool executed_;
};

class UnregisterEventHandlerAction : public IAction {
public:
    explicit UnregisterEventHandlerAction(Connection &conn, IEventHandler *handler);
    void execute(ServerState &state);

private:
    Connection &conn_;
    IEventHandler *handler_;
    bool executed_;
};

class RegisterEventAction : public IAction {
public:
    explicit RegisterEventAction(const Event &event);
    void execute(ServerState &state);

private:
    Event event_;
    bool executed_;
};

class UnregisterEventAction : public IAction {
public:
    explicit UnregisterEventAction(const Event &event);
    void execute(ServerState &state);

private:
    Event event_;
    bool executed_;
};

class TriggerPseudoEventAction : public IAction {
public:
    explicit TriggerPseudoEventAction(const Event &event);
    void execute(ServerState &state);

private:
    Event event_;
    bool executed_;
};

#endif
