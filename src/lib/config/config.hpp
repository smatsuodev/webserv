#ifndef SRC_LIB_CONFIG_CONFIG_HPP
#define SRC_LIB_CONFIG_CONFIG_HPP

#include "http/method.hpp"
#include "http/status.hpp"
#include "utils/types/option.hpp"
#include <stdint.h> // NOLINT(*-deprecated-headers)
#include <string>
#include <map>
#include <vector>

namespace config {
    class LocationContext {
    public:
        // root, autoindex, index からなるクラス
        class DocumentRootConfig {
        public:
            explicit DocumentRootConfig(
                const std::string &root, bool autoindex = false, const std::string &index = "index.html"
            );

        private:
            std::string root_;

        public:
            const std::string &getRoot() const;
            bool isAutoindex() const;
            const std::string &getIndex() const;

        private:
            bool autoindex_;
            std::string index_;
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

    class ServerContext {
    public:
        typedef std::map<http::HttpStatusCode, std::string> ErrorPageMap;

        explicit ServerContext(
            const std::vector<LocationContext> &locations,
            uint16_t port = 80,
            const std::string &host = "127.0.0.1",
            const std::vector<std::string> &serverName = std::vector<std::string>(),
            const ErrorPageMap &errorPage = ErrorPageMap(),
            std::size_t clientMaxBodySize = kDefaultClientMaxBodySize
        );

        const std::string &getHost() const;
        uint16_t getPort() const;
        std::size_t getClientMaxBodySize() const;
        const std::vector<std::string> &getServerName() const;
        const std::map<http::HttpStatusCode, std::string> &getErrorPage() const;
        const std::vector<LocationContext> &getLocations() const;

    private:
        static const std::size_t kDefaultClientMaxBodySize = 1048576; // 1 MiB

        std::string host_;
        uint16_t port_;
        std::size_t clientMaxBodySize_;
        std::vector<std::string> serverName_;
        ErrorPageMap errorPage_;
        std::vector<LocationContext> locations_;
    };

    class Config {
    public:
        explicit Config(const std::vector<ServerContext> &servers);

        const std::vector<ServerContext> &getServers() const;

    private:
        std::vector<ServerContext> servers_;
    };
}

#endif
