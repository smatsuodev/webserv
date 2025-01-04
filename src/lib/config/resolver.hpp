#ifndef SRC_LIB_CONFIG_RESOLVER_HPP
#define SRC_LIB_CONFIG_RESOLVER_HPP

#include "config.hpp"
#include "transport/address.hpp"

namespace config {
    // ip, port, Host ヘッダーから server config を解決する
    class Resolver {
    public:
        explicit Resolver(const Config &config);
        Option<ServerContext> resolve(const Address &address, const std::string &host) const;

    private:
        Config config_;
    };
}

#endif
