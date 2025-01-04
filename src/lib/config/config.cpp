#include "config.hpp"

namespace config {
    /* Config */
    Config::Config(const std::vector<ServerContext> &servers) : servers_(servers) {}

    Config::Config(const Config &other) : servers_(other.servers_) {}

    Config &Config::operator=(const Config &rhs) {
        if (this != &rhs) {
            servers_ = rhs.servers_;
        }
        return *this;
    }

    bool Config::operator==(const Config &rhs) const {
        return servers_ == rhs.servers_;
    }

    const std::vector<ServerContext> &Config::getServers() const {
        return servers_;
    }

    /* ServerContext */
    ServerContext::ServerContext(
        const std::string &host,
        const uint16_t port,
        const std::vector<LocationContext> &locations,
        const std::vector<std::string> &serverName,
        const ErrorPageMap &errorPage,
        const std::size_t clientMaxBodySize
    )
        : host_(host), port_(port), clientMaxBodySize_(clientMaxBodySize), serverName_(serverName),
          errorPage_(errorPage), locations_(locations) {}

    ServerContext::ServerContext(const ServerContext &other)
        : host_(other.host_), port_(other.port_), clientMaxBodySize_(other.clientMaxBodySize_),
          serverName_(other.serverName_), errorPage_(other.errorPage_), locations_(other.locations_) {}

    ServerContext &ServerContext::operator=(const ServerContext &rhs) {
        if (this != &rhs) {
            host_ = rhs.host_;
            port_ = rhs.port_;
            clientMaxBodySize_ = rhs.clientMaxBodySize_;
            serverName_ = rhs.serverName_;
            errorPage_ = rhs.errorPage_;
            locations_ = rhs.locations_;
        }
        return *this;
    }

    bool ServerContext::operator==(const ServerContext &rhs) const {
        return host_ == rhs.host_ && port_ == rhs.port_ && clientMaxBodySize_ == rhs.clientMaxBodySize_ &&
            serverName_ == rhs.serverName_ && errorPage_ == rhs.errorPage_ && locations_ == rhs.locations_;
    }

    const std::string &ServerContext::getHost() const {
        return host_;
    }

    uint16_t ServerContext::getPort() const {
        return port_;
    }

    std::size_t ServerContext::getClientMaxBodySize() const {
        return clientMaxBodySize_;
    }

    const std::vector<std::string> &ServerContext::getServerName() const {
        return serverName_;
    }

    const std::map<http::HttpStatusCode, std::string> &ServerContext::getErrorPage() const {
        return errorPage_;
    }

    const std::vector<LocationContext> &ServerContext::getLocations() const {
        return locations_;
    }

    /* LocationContext */
    LocationContext::LocationContext(
        const std::string &path, const DocumentRootConfig &docRootConfig, const AllowedMethods &allowedMethods
    )
        : path_(path), allowedMethods_(allowedMethods), docRootConfig_(Some(docRootConfig)), redirect_(None) {}

    LocationContext::LocationContext(
        const std::string &path, const std::string &redirect, const AllowedMethods &allowedMethods
    )
        : path_(path), allowedMethods_(allowedMethods), docRootConfig_(None), redirect_(Some(redirect)) {}

    LocationContext::LocationContext(const LocationContext &other)
        : path_(other.path_), allowedMethods_(other.allowedMethods_), docRootConfig_(other.docRootConfig_),
          redirect_(other.redirect_) {}

    LocationContext &LocationContext::operator=(const LocationContext &rhs) {
        if (this != &rhs) {
            path_ = rhs.path_;
            allowedMethods_ = rhs.allowedMethods_;
            docRootConfig_ = rhs.docRootConfig_;
            redirect_ = rhs.redirect_;
        }
        return *this;
    }

    bool LocationContext::operator==(const LocationContext &rhs) const {
        return path_ == rhs.path_ && allowedMethods_ == rhs.allowedMethods_ && redirect_ == rhs.redirect_ &&
            docRootConfig_ == rhs.docRootConfig_;
    }

    const std::string &LocationContext::getPath() const {
        return path_;
    }

    const std::vector<http::HttpMethod> &LocationContext::getAllowedMethods() const {
        return allowedMethods_;
    }

    const Option<std::string> &LocationContext::getRedirect() const {
        return redirect_;
    }

    const Option<LocationContext::DocumentRootConfig> &LocationContext::getDocumentRootConfig() const {
        return docRootConfig_;
    }

    /* LocationContext::DocumentRootConfig */
    LocationContext::DocumentRootConfig::DocumentRootConfig(
        const std::string &root, const bool autoindex, const std::string &index
    )
        : root_(root), autoindex_(autoindex), index_(index) {}

    LocationContext::DocumentRootConfig::DocumentRootConfig(const DocumentRootConfig &other)
        : root_(other.root_), autoindex_(other.autoindex_), index_(other.index_) {}

    LocationContext::DocumentRootConfig &LocationContext::DocumentRootConfig::operator=(const DocumentRootConfig &rhs) {
        if (this != &rhs) {
            root_ = rhs.root_;
            autoindex_ = rhs.autoindex_;
            index_ = rhs.index_;
        }
        return *this;
    }

    bool LocationContext::DocumentRootConfig::operator==(const DocumentRootConfig &rhs) const {
        return root_ == rhs.root_ && autoindex_ == rhs.autoindex_ && index_ == rhs.index_;
    }

    const std::string &LocationContext::DocumentRootConfig::getRoot() const {
        return root_;
    }

    bool LocationContext::DocumentRootConfig::isAutoindexEnabled() const {
        return autoindex_;
    }

    const std::string &LocationContext::DocumentRootConfig::getIndex() const {
        return index_;
    }

    LocationContext::AllowedMethods LocationContext::getDefaultAllowedMethods() {
        std::vector<http::HttpMethod> allowedMethods;
        allowedMethods.push_back(http::kMethodGet);
        return allowedMethods;
    }
}
