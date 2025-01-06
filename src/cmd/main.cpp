#include "config/config.hpp"
#include "core/server.hpp"
#include "http/handler/redirect_handler.hpp"

using namespace config;

// NOTE: 現状 http::Handler に設定を渡していないので、location は反映されていない
// 仮の設定。本当は設定ファイルから読み込む
Config loadConfig() {
    LocationContext location1(
        "/", LocationContext::DocumentRootConfig("./"), {http::kMethodGet, http::kMethodPost, http::kMethodDelete}
    );
    // src/ は GET だけ許可
    LocationContext location2("/src", LocationContext::DocumentRootConfig("./src"));
    LocationContext location3("/redirect", "https://example.com");

    return Config({ServerContext("0.0.0.0", 8080, {location1, location2, location3})});
}

int main() {
    const Config config = loadConfig();
    Server s(config);
    s.start();
}
