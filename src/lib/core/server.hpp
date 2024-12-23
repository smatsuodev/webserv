#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "transport/connection.hpp"

class Server {
public:
    Server();
    ~Server();
    void start(unsigned short port);

private:
    AutoFd epollFd_;

    void addToEpoll(int fd) const;
    void removeFromEpoll(int fd) const;
    static void setNonBlocking(int fd);

    enum HandleConnectionState {
        kSuspend,
        kComplete,
    };
    static Result<HandleConnectionState, error::AppError> handleConnection(const Connection &conn);
};

#endif
