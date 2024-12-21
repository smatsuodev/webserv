#ifndef SRC_LIB_LISTENER_HPP
#define SRC_LIB_LISTENER_HPP

#include <string>
#include <sys/socket.h>

#include "auto_close_fd.hpp"
#include "connection.hpp"

// サーバーソケットの抽象
class Listener {
public:
    Listener(const std::string &ip, unsigned short port, int backlog = SOMAXCONN);

    int getFd() const;
    Connection *acceptConnection() const;

private:
    AutoCloseFd serverFd_;

    static int setupSocket(const std::string &ip, unsigned short port, int backlog);
};

#endif
