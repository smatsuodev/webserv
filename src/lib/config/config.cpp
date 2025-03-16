#include "config.hpp"

#include "http/handler/middleware/logger.hpp"
#include "toml/parser.hpp"
#include "toml/value.hpp"
#include "utils/logger.hpp"

#include <fstream>
#include <iostream>

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

    Option<Config> Config::loadConfigFromFile(const std::string &path) {
        try {
            std::ifstream configFile(path);
            if (!configFile.is_open()) {
                LOG_ERRORF("cannot open config file: %s", path.c_str());
                return None;
            }

            std::stringstream buf;
            buf << configFile.rdbuf();
            std::string fileContent = buf.str();

            toml::Table configTable =
                toml::TomlParser(toml::Tokenizer(fileContent).tokenize().unwrap()).parse().unwrap();
            std::vector<toml::Value> serverConfigs =
                configTable.getValue("server").unwrap().getArray().unwrap().getElements();
            std::vector<ServerContext> servers;

            for (size_t i = 0; i < serverConfigs.size(); ++i) {
                toml::Table serverTable = serverConfigs[i].getTable().unwrap();
                servers.push_back(ServerContext::fromToml(serverTable));
            }

            return Some(Config(servers));
        } catch (const std::exception &e) {
            LOG_ERRORF("error while reading config: ", e.what());
            return None;
        }
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

    ServerContext ServerContext::fromToml(const toml::Table &serverTable) {
        std::string host = "0.0.0.0";
        if (serverTable.hasKey("host")) {
            host = serverTable.getValue("host").unwrap().getString().unwrap();
        }

        uint16_t port = 80;
        if (serverTable.hasKey("port")) {
            port = static_cast<uint16_t>(serverTable.getValue("port").unwrap().getInteger().unwrap());
        }

        std::vector<std::string> serverNames;
        if (serverTable.hasKey("server_name")) {
            std::vector<toml::Value> serverNameValues =
                serverTable.getValue("server_name").unwrap().getArray().unwrap().getElements();
            for (size_t j = 0; j < serverNameValues.size(); ++j) {
                serverNames.push_back(serverNameValues[j].getString().unwrap());
            }
        }

        ErrorPageMap errorPages;
        if (serverTable.hasKey("error_page")) {
            toml::Table errorPageTable = serverTable.getValue("error_page").unwrap().getTable().unwrap();
            std::vector<std::string> keys = errorPageTable.getKeys();
            for (size_t j = 0; j < keys.size(); ++j) {
                int statusCode = std::stoi(keys[j]);
                Option<http::HttpStatusCode> httpStatusCode = http::httpStatusCodeFromInt(statusCode);
                if (httpStatusCode.isNone()) {
                    LOG_WARNF("invalid HTTP status code: %d", statusCode);
                    continue;
                }
                std::string path = errorPageTable.getValue(keys[j]).unwrap().getString().unwrap();
                errorPages[httpStatusCode.unwrap()] = path;
            }
        }

        std::size_t clientMaxBodySize = kDefaultClientMaxBodySize;
        if (serverTable.hasKey("client_max_body_size")) {
            clientMaxBodySize = serverTable.getValue("client_max_body_size").unwrap().getInteger().unwrap();
        }

        std::vector<LocationContext> locations;
        if (serverTable.hasKey("location")) {
            std::vector<toml::Value> locationConfigs =
                serverTable.getValue("location").unwrap().getArray().unwrap().getElements();

            for (size_t j = 0; j < locationConfigs.size(); ++j) {
                toml::Table locationTable = locationConfigs[j].getTable().unwrap();
                locations.push_back(LocationContext::fromToml(locationTable));
            }
        }

        return ServerContext(host, port, locations, serverNames, errorPages, clientMaxBodySize);
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

    LocationContext LocationContext::fromToml(const toml::Table &locationTable) {
        std::string path = locationTable.getValue("path").unwrap().getString().unwrap();

        if (locationTable.hasKey("redirect")) {
            std::string redirect = locationTable.getValue("redirect").unwrap().getString().unwrap();
            return LocationContext(path, redirect);
        }

        std::string root = locationTable.getValue("root").unwrap().getString().unwrap();

        std::string index = "index.html";
        if (locationTable.hasKey("index")) {
            index = locationTable.getValue("index").unwrap().getString().unwrap();
        }

        bool autoindex = false;
        if (locationTable.hasKey("autoindex")) {
            std::string autoindexValue = locationTable.getValue("autoindex").unwrap().getString().unwrap();
            autoindex = (autoindexValue == "on");
        }

        std::vector<http::HttpMethod> allowedMethods = getDefaultAllowedMethods();
        if (locationTable.hasKey("allowed_methods")) {
            std::vector<toml::Value> methodValues =
                locationTable.getValue("allowed_methods").unwrap().getArray().unwrap().getElements();
            allowedMethods.clear();

            for (size_t k = 0; k < methodValues.size(); ++k) {
                std::string methodStr = methodValues[k].getString().unwrap();
                http::HttpMethod method = http::httpMethodFromString(methodStr);
                if (method == http::HttpMethod::kMethodUnknown) {
                    LOG_ERRORF("unknown http method: %s", methodStr.c_str());
                    throw std::runtime_error("unknown method: " + methodStr);
                }
                allowedMethods.push_back(method);
            }
        }

        DocumentRootConfig docRootConfig(root, autoindex, index);
        return LocationContext(path, docRootConfig, allowedMethods);
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
