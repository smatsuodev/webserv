#ifndef SRC_LIB_CORE_VIRTUAL_SERVER_HPP
#define SRC_LIB_CORE_VIRTUAL_SERVER_HPP

#include "config/config.hpp"
#include "http/handler/router.hpp"
#include "transport/address.hpp"

class VirtualServer {
public:
    explicit VirtualServer(const config::ServerContext &serverConfig);

    const config::ServerContext &getServerConfig() const;
    http::Router &getRouter();

    bool operator==(const VirtualServer &rhs) const;
    void registerHandlers(const config::LocationContext &location);

private:
    config::ServerContext serverConfig_;
    http::Router router_;

    void setupRouter();

    // Virtual Server の検索に必要
    friend class VirtualServerResolver;
    bool isMatch(const Address &address) const;
};

#endif
