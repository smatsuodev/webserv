#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "connection.hpp"

class Server {
public:
    Server();
    ~Server();
    static void start(unsigned short port);

private:
    static void handleConnection(const Connection &conn);
};

#endif
