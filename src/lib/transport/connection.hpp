#ifndef SRC_LIB_CONNECTION_HPP
#define SRC_LIB_CONNECTION_HPP

#include "endpoint.hpp"
#include "utils/auto_fd.hpp"
#include "utils/io/read_buffer.hpp"
#include "utils/io/reader.hpp"

// クライアントソケットの抽象
class Connection {
public:
    Connection(int fd, const Endpoint &listenerEndpoint, const Endpoint &clientEndpoint);
    ~Connection();

    int getFd() const;
    const Endpoint &getListenerEndpoint() const;
    const Endpoint &getClientEndpoint() const;
    ReadBuffer &getReadBuffer();

private:
    AutoFd clientFd_;
    Endpoint listenerEndpoint_;
    Endpoint clientEndpoint_;
    io::FdReader fdReader_; // ReadBuffer に渡す IReader & の参照先として必要
    ReadBuffer buffer_;
};

#endif
