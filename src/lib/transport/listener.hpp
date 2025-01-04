#ifndef SRC_LIB_LISTENER_HPP
#define SRC_LIB_LISTENER_HPP

#include <string>
#include <sys/socket.h>
#include "utils/auto_fd.hpp"
#include "connection.hpp"
#include "endpoint.hpp"
#include "utils/types/result.hpp"

// サーバーソケットの抽象
class Listener {
public:
    explicit Listener(const Endpoint &endpoint, int backlog = SOMAXCONN);
    ~Listener();

    int getFd() const;
    const Endpoint &getEndpoint() const;

    typedef Result<Connection *, std::string> AcceptConnectionResult;
    AcceptConnectionResult acceptConnection() const;

private:
    AutoFd serverFd_;
    Endpoint endpoint_;

    static int setupSocket(const std::string &ip, unsigned short port, int backlog);
};

#endif
