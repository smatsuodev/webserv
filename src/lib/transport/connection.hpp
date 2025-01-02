#ifndef SRC_LIB_CONNECTION_HPP
#define SRC_LIB_CONNECTION_HPP

#include "utils/auto_fd.hpp"
#include "utils/io/read_buffer.hpp"
#include "utils/io/reader.hpp"
#include <memory>

// クライアントソケットの抽象
class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    int getFd() const;
    ReadBuffer &getReadBuffer();

private:
    AutoFd clientFd_;
    io::FdReader fdReader_; // ReadBuffer に渡す IReader & の参照先として必要
    ReadBuffer buffer_;
};

#endif
