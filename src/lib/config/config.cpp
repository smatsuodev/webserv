#include "config.hpp"

namespace config {
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

    ServerContext::ServerContext(const std::vector<LocationContext> &locations, const std::string &host,
                                 const uint16_t port, const std::size_t clientMaxBodySize,
                                 const std::vector<std::string> &serverName, const ErrorPageMap &errorPage)
        : host_(host), port_(port), clientMaxBodySize_(clientMaxBodySize), serverName_(serverName),
          errorPage_(errorPage), locations_(locations) {}

    Config::Config(const std::vector<ServerContext> &servers) : servers_(servers) {}
}
