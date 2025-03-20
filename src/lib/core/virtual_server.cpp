#include "virtual_server.hpp"
#include "http/handler/delete_file_handler.hpp"
#include "http/handler/redirect_handler.hpp"
#include "http/handler/static_file_handler.hpp"
#include "http/handler/upload_file_handler.hpp"
#include "http/handler/middleware/error_page.hpp"
#include "http/handler/middleware/logger.hpp"
#include "utils/logger.hpp"

VirtualServer::VirtualServer(const config::ServerContext &serverConfig, const Address &bindAddress)
    : serverConfig_(serverConfig), bindAddress_(bindAddress) {
    LOG_DEBUGF("<-- setup virtual server for %s", bindAddress.toString().c_str());
    this->setupRouter();
    LOG_DEBUGF("--> setup complete");
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
    std::vector<http::HttpMethod> allowedMethods = location.getAllowedMethods();
    for (std::vector<http::HttpMethod>::const_iterator iter = allowedMethods.begin(); iter != allowedMethods.end();
         ++iter) {
        config::LocationContext::DocumentRootConfig documentRootConfig = location.getDocumentRootConfig().unwrap();
        switch (*iter) {
            case http::kMethodGet: {
                http::IHandler *handler = new http::StaticFileHandler(documentRootConfig);
                router_.on(http::kMethodGet, location.getPath(), handler);
                break;
            }
            case http::kMethodDelete: {
                http::IHandler *handler = new http::DeleteFileHandler(documentRootConfig);
                router_.on(http::kMethodDelete, location.getPath(), handler);
                break;
            }
            case http::kMethodPost: {
                http::IHandler *handler = new http::UploadFileHandler(documentRootConfig);
                router_.on(http::kMethodPost, location.getPath(), handler);
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

    // middleware
    router_.use(new http::Logger());
    router_.use(new http::ErrorPage(serverConfig_.getErrorPage()));
}

bool VirtualServer::isMatch(const Address &address) const {
    return bindAddress_.getPort() == address.getPort() &&
        (bindAddress_.getIp() == "0.0.0.0" || bindAddress_.getIp() == address.getIp());
}
