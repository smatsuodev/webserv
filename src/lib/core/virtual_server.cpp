#include "virtual_server.hpp"
#include "http/handler/delete_file_handler.hpp"
#include "http/handler/redirect_handler.hpp"
#include "http/handler/static_file_handler.hpp"
#include "utils/logger.hpp"

VirtualServer::VirtualServer(const config::ServerContext &serverConfig) : serverConfig_(serverConfig) {
    this->setupRouter();
}

const config::ServerContext &VirtualServer::getServerConfig() const {
    return serverConfig_;
}

http::Router &VirtualServer::getRouter() {
    return router_;
}

// NOTE: テストなどに必要。設定に対して http::Router は一意なので、serverConfig_ だけで比較している。
bool VirtualServer::operator==(const VirtualServer &rhs) const {
    return serverConfig_ == rhs.serverConfig_;
}

void VirtualServer::registerHandlers(const config::LocationContext &location) {
    // TODO: 設定を渡す
    std::vector<http::HttpMethod> allowedMethods = location.getAllowedMethods();
    for (std::vector<http::HttpMethod>::const_iterator iter = allowedMethods.begin(); iter != allowedMethods.end();
         ++iter) {
        config::LocationContext::DocumentRootConfig documentRootConfig = location.getDocumentRootConfig().unwrap();
        switch (*iter) {
            case http::kMethodGet: {
                LOG_DEBUGF("register GET handler: %s", location.getPath().c_str());
                http::IHandler *handler = new http::StaticFileHandler(documentRootConfig);
                router_.onGet(location.getPath(), handler);
                break;
            }
            case http::kMethodDelete: {
                LOG_DEBUGF("register DELETE handler: %s", location.getPath().c_str());
                http::IHandler *handler = new http::DeleteFileHandler(documentRootConfig);
                router_.onDelete(location.getPath(), handler);
                break;
            }
            default:
                // do nothing
                break;
        }
    }
}

void VirtualServer::setupRouter() {
    config::LocationContextList locations = serverConfig_.getLocations();
    for (config::LocationContextList::const_iterator it = locations.begin(); it != locations.end(); ++it) {
        const config::LocationContext &location = *it;
        if (location.getRedirect().isSome()) {
            http::IHandler *handler = new http::RedirectHandler(location.getRedirect().unwrap());
            router_.on(location.getAllowedMethods(), location.getPath(), handler);
        } else {
            this->registerHandlers(location);
        }
    }
}

bool VirtualServer::isMatch(const Address &address) const {
    return serverConfig_.getPort() == address.getPort() &&
        (serverConfig_.getHost() == "0.0.0.0" || serverConfig_.getHost() == address.getIp());
}
