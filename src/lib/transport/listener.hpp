#ifndef SRC_LIB_LISTENER_HPP
#define SRC_LIB_LISTENER_HPP

#include <string>
#include <sys/socket.h>
#include "utils/auto_fd.hpp"
#include "connection.hpp"
#include "address.hpp"
#include "utils/types/result.hpp"

// サーバーソケットの抽象
class Listener {
public:
    explicit Listener(const Address &listenAddress, int backlog = SOMAXCONN);
    ~Listener();

    int getFd() const;

    typedef Result<Connection *, std::string> AcceptConnectionResult;
    AcceptConnectionResult acceptConnection() const;

private:
    AutoFd serverFd_;

    static int setupSocket(const Address &listenAddress, int backlog);
};

#endif
