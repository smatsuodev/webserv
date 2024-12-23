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

    static void handleConnection(const Connection &conn);
    void addToEpoll(int fd) const;
    static void setNonBlocking(int fd);
};

#endif
