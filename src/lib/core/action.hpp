#ifndef SRC_LIB_CORE_ACTION_HPP
#define SRC_LIB_CORE_ACTION_HPP

#include "server.hpp"

// IEventHandler が返す Command オブジェクト
class IAction {
public:
    virtual ~IAction();
    virtual void execute(Server &server) = 0;
};

class AddConnectionAction : public IAction {
public:
    explicit AddConnectionAction(Connection *conn);
    void execute(Server &server);

private:
    Connection *conn_;
};

class RemoveConnectionAction : public IAction {
public:
    explicit RemoveConnectionAction(Connection *conn);
    void execute(Server &server);

private:
    Connection *conn_;
};

class UnregisterEventHandlerAction : public IAction {
public:
    explicit UnregisterEventHandlerAction(Connection *conn, IEventHandler *handler);
    void execute(Server &server);

private:
    Connection *conn_;
    IEventHandler *handler_;
    bool executed_;
};

class RegisterEventAction : public IAction {
public:
    explicit RegisterEventAction(const Event &event);
    void execute(Server &server);

private:
    Event event_;
    bool executed_;
};

class UnregisterEventAction : public IAction {
public:
    explicit UnregisterEventAction(const Event &event);
    void execute(Server &server);

private:
    Event event_;
    bool executed_;
};

class RegisterEventHandlerAction : public IAction {
public:
    RegisterEventHandlerAction(Connection *conn, IEventHandler *handler);
    void execute(Server &server);

private:
    Connection *conn_;
    IEventHandler *handler_;
    bool executed_;
};

#endif
