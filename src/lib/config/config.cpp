#include "config.hpp"

namespace config {
    const std::string &LocationContext::getPath() const {
        return path_;
    }

    const std::string &LocationContext::getRoot() const {
        return root_;
    }

    const std::string &LocationContext::getIndex() const {
        return index_;
    }

    bool LocationContext::isAutoindexEnabled() const {
        return autoindex_;
    }

    const std::vector<http::HttpMethod> &LocationContext::getAllowedMethods() const {
        return allowedMethods_;
    }

    const Option<std::string> &LocationContext::getRedirect() const {
        return redirect_;
    }

    LocationContext::LocationContext(const std::string &path, const std::string &root, const std::string &index,
                                     const bool autoindex, const std::vector<http::HttpMethod> &allowedMethods,
                                     const Option<std::string> &redirect)
        : path_(path), root_(root), index_(index), autoindex_(autoindex), allowedMethods_(allowedMethods),
          redirect_(redirect) {}

    std::vector<http::HttpMethod> LocationContext::getDefaultAllowedMethods() {
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

    ServerContext::ServerContext(const std::vector<LocationContext> &locations, const std::string &host,
                                 const uint16_t port, const std::size_t clientMaxBodySize,
                                 const std::vector<std::string> &serverName, const ErrorPageMap &errorPage)
        : host_(host), port_(port), clientMaxBodySize_(clientMaxBodySize), serverName_(serverName),
          errorPage_(errorPage), locations_(locations) {}

    const std::vector<ServerContext> &Config::getServers() const {
        return servers_;
    }

    Config::Config(const std::vector<ServerContext> &servers) : servers_(servers) {}
}
