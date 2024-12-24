#ifndef SRC_LIB_SERVER_HPP
#define SRC_LIB_SERVER_HPP

#include "transport/connection.hpp"

class Server {
public:
    Server();
    ~Server();
    static void start(unsigned short port);

private:
    enum HandleConnectionState {
        kSuspend,
        kComplete,
    };
    static Result<HandleConnectionState, error::AppError> handleConnection(const Connection &conn);
};

#endif
