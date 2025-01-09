#ifndef SRC_LIB_CORE_VIRTUAL_SERVER_RESOLVER_HPP
#define SRC_LIB_CORE_VIRTUAL_SERVER_RESOLVER_HPP

#include "virtual_server.hpp"
#include "event/event_handler.hpp"
#include "transport/connection.hpp"
#include "utils/ref.hpp"

// 本当は Ref が好ましいが、Server がポインタで持っているので、それに合わせる
typedef std::vector<VirtualServer *> VirtualServerList;

/**
 * NOTE:
 * Server からは HTTP レイヤーの情報が得られないため、Context 経由でイベントハンドラーに渡して Virtual Server
 * を解決させる
 */
class VirtualServerResolver {
public:
    explicit VirtualServerResolver(const VirtualServerList &virtualServers, const Ref<const Connection> &conn);
    Option<Ref<VirtualServer> > resolve(const std::string &hostHeader) const;

private:
    VirtualServerList virtualServers_;
    Ref<const Connection> conn_;
};

class VirtualServerResolverFactory : public IVirtualServerResolverFactory {
public:
    explicit VirtualServerResolverFactory(const VirtualServerList &virtualServers);
    VirtualServerResolver create(const Ref<Connection> &conn);

private:
    VirtualServerList virtualServers_;
};

#endif
