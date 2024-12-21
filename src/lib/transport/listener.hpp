#ifndef SRC_LIB_LISTENER_HPP
#define SRC_LIB_LISTENER_HPP

#include <string>
#include <sys/socket.h>
#include "utils/auto_close_fd.hpp"
#include "connection.hpp"
#include "utils/result.hpp"

// サーバーソケットの抽象
class Listener {
public:
    Listener(const std::string &ip, unsigned short port, int backlog = SOMAXCONN);
    ~Listener();

    int getFd() const;

    using AcceptConnectionResult = Result<Connection *, std::string>;
    AcceptConnectionResult acceptConnection() const;

private:
    AutoCloseFd serverFd_;

    static int setupSocket(const std::string &ip, unsigned short port, int backlog);
};

#endif
