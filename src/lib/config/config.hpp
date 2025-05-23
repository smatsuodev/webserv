#ifndef SRC_LIB_CONFIG_CONFIG_HPP
#define SRC_LIB_CONFIG_CONFIG_HPP

#include "http/method.hpp"
#include "http/status.hpp"
#include "toml/value.hpp"
#include "utils/types/option.hpp"
#include <stdint.h> // NOLINT(*-deprecated-headers)
#include <string>
#include <map>
#include <vector>

namespace config {
    class LocationContext;
    class ServerContext;

    typedef std::vector<ServerContext> ServerContextList;
    typedef std::vector<LocationContext> LocationContextList;

    class Config {
    public:
        explicit Config(const ServerContextList &servers);
        Config(const Config &other);

        Config &operator=(const Config &rhs);
        bool operator==(const Config &rhs) const;

        static Option<Config> loadConfigFromFile(const std::string &path);

        const ServerContextList &getServers() const;

    private:
        ServerContextList servers_;
    };

    class ServerContext {
    public:
        typedef std::map<http::HttpStatusCode, std::string> ErrorPageMap;

        explicit ServerContext(
            const std::string &host,
            uint16_t port,
            const LocationContextList &locations,
            const std::vector<std::string> &serverName = std::vector<std::string>(),
            const ErrorPageMap &errorPage = ErrorPageMap(),
            std::size_t clientMaxBodySize = kDefaultClientMaxBodySize
        );
        ServerContext(const ServerContext &other);

        ServerContext &operator=(const ServerContext &rhs);
        bool operator==(const ServerContext &rhs) const;

        static ServerContext fromToml(const toml::Table &serverTable);

        const std::string &getHost() const;
        uint16_t getPort() const;
        std::size_t getClientMaxBodySize() const;
        const std::vector<std::string> &getServerName() const;
        const std::map<http::HttpStatusCode, std::string> &getErrorPage() const;
        const LocationContextList &getLocations() const;

    private:
        static const std::size_t kDefaultClientMaxBodySize = 1048576; // 1 MiB
        std::string host_;
        uint16_t port_;
        std::size_t clientMaxBodySize_;
        std::vector<std::string> serverName_;
        ErrorPageMap errorPage_;
        LocationContextList locations_;
    };

    class LocationContext {
    public:
        class DocumentRootConfig {
        public:
            explicit DocumentRootConfig(
                const std::string &root,
                bool autoindex = false,
                const std::string &index = "index.html",
                const std::vector<std::string> &cgiExtensions = std::vector<std::string>()
            );
            DocumentRootConfig(const DocumentRootConfig &other);

            DocumentRootConfig &operator=(const DocumentRootConfig &rhs);
            bool operator==(const DocumentRootConfig &rhs) const;

            const std::string &getRoot() const;
            bool isAutoindexEnabled() const;
            const std::string &getIndex() const;
            const std::vector<std::string> &getCgiExtensions() const;

        private:
            std::string root_;
            bool autoindex_;
            std::string index_;
            std::vector<std::string> cgiExtensions_;
        };

        typedef std::vector<http::HttpMethod> AllowedMethods;

        // NOTE: allowedMethods の初期値を指定したいが、C++98 で初期化子リストが使えない
        LocationContext(
            const std::string &path,
            const DocumentRootConfig &docRootConfig,
            const AllowedMethods &allowedMethods = getDefaultAllowedMethods()
        );
        LocationContext(
            const std::string &path,
            const std::string &redirect,
            const AllowedMethods &allowedMethods = getDefaultAllowedMethods()
        );
        LocationContext(const LocationContext &other);

        LocationContext &operator=(const LocationContext &rhs);
        bool operator==(const LocationContext &rhs) const;

        static LocationContext fromToml(const toml::Table &locationTable);

        const std::string &getPath() const;
        const std::vector<http::HttpMethod> &getAllowedMethods() const;
        const Option<std::string> &getRedirect() const;
        const Option<DocumentRootConfig> &getDocumentRootConfig() const;

    private:
        std::string path_;
        std::vector<http::HttpMethod> allowedMethods_;
        Option<DocumentRootConfig> docRootConfig_;
        Option<std::string> redirect_;

        static AllowedMethods getDefaultAllowedMethods();
    };
}

#endif
