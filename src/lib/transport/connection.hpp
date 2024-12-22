#ifndef SRC_LIB_CONNECTION_HPP
#define SRC_LIB_CONNECTION_HPP

#include "utils/auto_fd.hpp"
#include "utils/io/reader.hpp"
#include <memory>

// クライアントソケットの抽象
class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    int getFd() const;
    io::IReader &getReader() const;

private:
    AutoFd clientFd_;
    std::auto_ptr<io::IReader> reader_;
};

#endif
