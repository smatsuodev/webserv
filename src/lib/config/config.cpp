#include "config.hpp"

namespace config {
    LocationContext::LocationContext(
        const std::string &path, const DocumentRootConfig &docRootConfig, const AllowedMethods &allowedMethods
    )
        : path_(path), allowedMethods_(allowedMethods), docRootConfig_(Some(docRootConfig)), redirect_(None) {}

    LocationContext::LocationContext(
        const std::string &path, const std::string &redirect, const AllowedMethods &allowedMethods
    )
        : path_(path), allowedMethods_(allowedMethods), docRootConfig_(None), redirect_(Some(redirect)) {}

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

    LocationContext::DocumentRootConfig::DocumentRootConfig(
        const std::string &root, const bool autoindex, const std::string &index
    )
        : root_(root), autoindex_(autoindex), index_(index) {}

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

    ServerContext::ServerContext(
        const std::vector<LocationContext> &locations,
        const uint16_t port,
        const std::string &host,
        const std::vector<std::string> &serverName,
        const ErrorPageMap &errorPage,
        const std::size_t clientMaxBodySize
    )
        : host_(host), port_(port), clientMaxBodySize_(clientMaxBodySize), serverName_(serverName),
          errorPage_(errorPage), locations_(locations) {}

    const std::vector<ServerContext> &Config::getServers() const {
        return servers_;
    }

    Config::Config(const std::vector<ServerContext> &servers) : servers_(servers) {}
}
