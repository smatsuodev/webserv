#ifndef SRC_LIB_CORE_VIRTUAL_SERVER_HPP
#define SRC_LIB_CORE_VIRTUAL_SERVER_HPP

#include "config/config.hpp"
#include "http/handler/router.hpp"
#include "transport/address.hpp"

class VirtualServer {
public:
    // config はホスト名で書かれる場合があるので、解決済みのアドレスももらう
    explicit VirtualServer(const config::ServerContext &serverConfig, const Address &bindAddress);

    const config::ServerContext &getServerConfig() const;
    http::Router &getRouter();

    bool operator==(const VirtualServer &rhs) const;
    void registerHandlers(const config::LocationContext &location);

private:
    config::ServerContext serverConfig_;
    Address bindAddress_;
    http::Router router_;

    void setupRouter();

    // Virtual Server の検索に必要
    friend class VirtualServerResolver;
    bool isMatch(const Address &address) const;
};

#endif
