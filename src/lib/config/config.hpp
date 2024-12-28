#ifndef SRC_LIB_CONFIG_CONFIG_HPP
#define SRC_LIB_CONFIG_CONFIG_HPP

#include "http/method.hpp"
#include "http/status.hpp"
#include "utils/types/option.hpp"

#include <stdint.h>
#include <string>
#include <map>
#include <vector>

namespace config {
    class LocationContext {
    public:
        // NOTE: allowedMethods の初期値を指定したいが、C++98 で初期化子リストが使えない
        LocationContext(const std::string &path, const std::string &root, const std::string &index,
                        bool autoindex = false,
                        const std::vector<http::HttpMethod> &allowedMethods = getDefaultAllowedMethods(),
                        const Option<std::string> &redirect = None);

    private:
        std::string path_;
        std::string root_;
        std::string index_;
        bool autoindex_;
        std::vector<http::HttpMethod> allowedMethods_;
        Option<std::string> redirect_;

        static std::vector<http::HttpMethod> getDefaultAllowedMethods();
    };

    class ServerContext {
    public:
        typedef std::map<http::HttpStatusCode, std::string> ErrorPageMap;

        explicit ServerContext(const std::vector<LocationContext> &locations, const std::string &host = "0.0.0.0",
                               uint16_t port = 80, std::size_t clientMaxBodySize = kDefaultClientMaxBodySize,
                               const std::vector<std::string> &serverName = std::vector<std::string>(),
                               const ErrorPageMap &errorPage = ErrorPageMap());

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

    private:
        std::vector<ServerContext> servers_;
    };
}

#endif
