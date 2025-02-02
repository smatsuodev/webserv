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
    explicit Listener(const std::string &host, const std::string &port, int backlog = SOMAXCONN);
    ~Listener();

    int getFd() const;
    const Address &getBindAddress() const;

    typedef Result<Connection *, std::string> AcceptConnectionResult;
    AcceptConnectionResult acceptConnection() const;

private:
    AutoFd serverFd_;
    Address bindAddress_;

    void setupSocket(const std::string &host, const std::string &port, int backlog);
};

#endif
