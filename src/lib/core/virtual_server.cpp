#include "virtual_server.hpp"
#include "http/handler/redirect_handler.hpp"

VirtualServer::VirtualServer(const config::ServerContext &serverConfig) : serverConfig_(serverConfig) {
    this->setupRouter();
}

const config::ServerContext &VirtualServer::getServerConfig() const {
    return serverConfig_;
}

http::Router &VirtualServer::getRouter() {
    return router_;
}

void VirtualServer::setupRouter() {
    config::LocationContextList locations = serverConfig_.getLocations();
    for (config::LocationContextList::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const config::LocationContext &location = *it;
        http::IHandler *handler = NULL;
        if (location.getRedirect().isSome()) {
            handler = new http::RedirectHandler(location.getRedirect().unwrap());
        } else {
            // TODO: 設定を渡す
            handler = new http::Handler();
        }
        router_.on(location.getAllowedMethods(), location.getPath(), handler);
    }
}

bool VirtualServer::isMatch(const Address &address) const {
    return serverConfig_.getPort() == address.getPort() &&
        (serverConfig_.getHost() == "0.0.0.0" || serverConfig_.getHost() == address.getIp());
}

