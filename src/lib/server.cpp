#include "server.hpp"

#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include "listener.hpp"
#include "utils/string.hpp"
#include "utils/logger.hpp"

Server::Server() {}

Server::~Server() {}

void Server::start(const unsigned short port) {
    const Listener lsn("0.0.0.0", port);

    LOG_INFO(utils::format("server started on port %u", port));

    while (true) {
        const Connection *conn = lsn.acceptConnection();
        if (conn == NULL) {
            LOG_WARN("failed to accept connection");
            continue;
        }

        handleConnection(*conn);

        delete conn;
    }
}

void Server::handleConnection(const Connection &conn) {
    // 適当なHTTPレスポンスを書き込む
    const std::string responseString =
        "HTTP/1.1 200 OK\r\n"
        "Content-Length: 14\r\n"
        "Content-Type: text/plain\r\n"
        "\r\n"
        "Hello, World!\n";

    if (send(conn.getFd(), responseString.c_str(), responseString.size(), 0) == -1) {
        LOG_WARN("failed to send response");
        return;
    }

    LOG_INFO("response sent");
}
