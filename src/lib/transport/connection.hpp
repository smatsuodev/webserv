#ifndef SRC_LIB_CONNECTION_HPP
#define SRC_LIB_CONNECTION_HPP

#include "address.hpp"
#include "utils/auto_fd.hpp"
#include "utils/io/read_buffer.hpp"
#include "utils/io/reader.hpp"
#include <ctime>

// クライアントソケットの抽象
class Connection {
public:
    Connection(int fd, const Address &localAddress, const Address &foreignAddress);
    ~Connection();

    int getFd() const;
    const Address &getLocalAddress() const;
    const Address &getForeignAddress() const;
    ReadBuffer &getReadBuffer();
    std::time_t getLastActivityTime() const;
    void updateActivity();

private:
    AutoFd clientFd_;
    Address localAddress_;
    Address foreignAddress_;
    io::FdReader fdReader_; // ReadBuffer に渡す IReader & の参照先として必要
    ReadBuffer buffer_;
    std::time_t lastActivityTime_;
};

#endif
