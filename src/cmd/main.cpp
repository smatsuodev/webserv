#include <exception>
#include "config/config.hpp"
#include "core/server.hpp"
#include "utils/logger.hpp"

using namespace config;

int startServer(const int argc, char *argv[]) {
    if (argc != 2) {
        LOG_ERROR("Usage: webserv <config file>");
        return 1;
    }

    SET_LOG_LEVEL(Logger::kInfo);

    const Option<Config> config = Config::loadConfigFromFile(argv[1]);
    if (config.isNone()) {
        return 1;
    }
    Server s(config.unwrap());
    s.start();
    return 0;
}

int main(const int argc, char *argv[]) {
    while (true) {
        try {
            return startServer(argc, argv);
        } catch (std::exception &e) {
        }
    }
}
