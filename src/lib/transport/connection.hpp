#ifndef SRC_LIB_CONNECTION_HPP
#define SRC_LIB_CONNECTION_HPP

#include "utils/auto_fd.hpp"

// クライアントソケットの抽象
class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    int getFd() const;

private:
    AutoFd clientFd_;
};

#endif
