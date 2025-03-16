#include "config/config.hpp"
#include "core/server.hpp"
#include "http/handler/redirect_handler.hpp"
#include "utils/logger.hpp"

using namespace config;

// NOTE: 現状 http::Handler に設定を渡していないので、location は反映されていない
// 仮の設定。本当は設定ファイルから読み込む
Config loadConfig() {
    LocationContext location1(
        "/", LocationContext::DocumentRootConfig("./", true), {http::kMethodGet, http::kMethodPost, http::kMethodDelete}
    );
    // src/ は GET だけ許可。autoindex も有効化しない
    LocationContext location2("/src", LocationContext::DocumentRootConfig("./"));
    ServerContext server1("localhost", 8080, {location1, location2});

    LocationContext location3("/", LocationContext::DocumentRootConfig("./example/html", true));
    LocationContext location4("/other", LocationContext::DocumentRootConfig("./example/html", true, "home.html"));
    ServerContext server2("0.0.0.0", 8081, {location3, location4});

    // 同じ 8081 ポートで別のサーバーを定義
    // 全部 example.com にリダイレクト
    ServerContext server3("0.0.0.0", 8081, {LocationContext("/", "https://example.com")}, {"redirect.example.com"});

    return Config({server1, server2, server3});
}

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
