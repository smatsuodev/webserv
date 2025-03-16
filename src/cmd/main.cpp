#include "config/config.hpp"
#include "core/server.hpp"
#include "http/handler/redirect_handler.hpp"
#include "utils/logger.hpp"

using namespace config;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        LOG_ERROR("Usage: ./webserv <config file>");
        return 1;
    }

    const Option<Config> config = Config::loadConfigFromFile(argv[1]);
    if (config.isNone()) {
        return 1;
    }
    Server s(config.unwrap());
    s.start();
    return 0;
}
