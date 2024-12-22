#include "server.hpp"
#include <cstdio>
#include <iostream>
#include <sys/socket.h>
#include "transport/listener.hpp"
#include "utils/string.hpp"
#include "utils/logger.hpp"
#include <string>

Server::Server() {}

Server::~Server() {}

void Server::start(const unsigned short port) {
    const Listener lsn("0.0.0.0", port);

    LOG_INFOF("server started on port %u", port);

    while (true) {
        const Listener::AcceptConnectionResult result = lsn.acceptConnection();
        if (result.isErr()) {
            LOG_WARN(result.unwrapErr());
            continue;
        }

        const Connection *conn = result.unwrap();
        handleConnection(*conn);

        delete conn;
    }
}

void Server::handleConnection(const Connection &conn) {
    bufio::Reader reader(conn.getReader());

    const bufio::Reader::ReadAllResult result = reader.readAll();
    if (result.isErr()) {
        LOG_WARN(result.unwrapErr());
        return;
    }
    const std::string content = result.unwrap();

    if (send(conn.getFd(), content.c_str(), content.size(), 0) == -1) {
        LOG_WARN("failed to send response");
        return;
    }

    LOG_DEBUG("response sent");
}
