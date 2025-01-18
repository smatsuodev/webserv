#include "config/config.hpp"
#include "core/server.hpp"
#include "http/handler/redirect_handler.hpp"

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

int main() {
    const Config config = loadConfig();
    Server s(config);
    s.start();
}
