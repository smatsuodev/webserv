#include <gtest/gtest.h>
#include "config/resolver.hpp"
#include "transport/address.hpp"

using namespace config;

class ResolverTest : public ::testing::Test {
protected:
    static ServerContext createServer(
        const std::string &host,
        const uint16_t port,
        const std::vector<std::string> &serverName = std::vector<std::string>()
    ) {
        return ServerContext(host, port, LocationContextList(), serverName);
    }
};

TEST_F(ResolverTest, ResolveByAddress) {
    ServerContextList servers;
    servers.push_back(createServer("127.0.0.1", 8080));
    servers.push_back(createServer("127.0.0.1", 8081));

    const Config config(servers);
    const Resolver resolver(config);

    const Address addr("127.0.0.1", 8080);
    const auto result = resolver.resolve(addr, "");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), servers[0]);

    const Address addr2("127.0.0.1", 8081);
    const auto result2 = resolver.resolve(addr2, "");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap(), servers[1]);

    // 登録していないアドレス
    const Address addr3("192.168.0.1", 8080);
    const auto result3 = resolver.resolve(addr3, "");
    EXPECT_TRUE(result3.isNone());

    // 登録していないポート
    const Address nonExistAddr("127.0.0.1", 9999);
    const auto result4 = resolver.resolve(nonExistAddr, "");
    EXPECT_TRUE(result4.isNone());
}

TEST_F(ResolverTest, ResolveByHostHeader) {
    ServerContextList servers;
    servers.push_back(createServer("127.0.0.1", 80, {"example.org"}));
    servers.push_back(createServer("127.0.0.1", 80, {"example.com", "www.example.com"}));

    const Config config(servers);
    const Resolver resolver(config);

    // 127.0.0.1:80 は複数あるが、Host が一致するのは前者
    const Address addr("127.0.0.1", 80);
    const auto result = resolver.resolve(addr, "example.com");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), servers[1]);

    const auto result2 = resolver.resolve(addr, "example.org");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap(), servers[0]);

    // Host が一致しない場合は先頭のものが選択される
    const auto result3 = resolver.resolve(addr, "unknown.com");
    ASSERT_TRUE(result3.isSome());
    EXPECT_EQ(result3.unwrap(), servers[0]);
}

TEST_F(ResolverTest, ResolveWithWildcard) {
    ServerContextList servers;
    servers.push_back(createServer("127.0.0.1", 80, {"example.org"}));
    servers.push_back(createServer("0.0.0.0", 80, {"example.com"}));

    const Config config(servers);
    const Resolver resolver(config);

    // ワイルドカードより具体的なアドレスが優先される
    const Address addr("127.0.0.1", 80);
    const auto result = resolver.resolve(addr, "");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), servers[0]);

    // Host を指定しても同様に優先される
    const auto result2 = resolver.resolve(addr, "example.com");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap(), servers[0]);

    // 具体的なアドレスがない場合はワイルドカードにマッチ
    const Address addr2("192.168.0.1", 80);
    const auto result3 = resolver.resolve(addr2, "");
    ASSERT_TRUE(result3.isSome());
    EXPECT_EQ(result3.unwrap(), servers[1]);
}

TEST_F(ResolverTest, ResolveWithMultipleWildcards) {
    ServerContextList servers;
    servers.push_back(createServer("0.0.0.0", 80));
    servers.push_back(createServer("0.0.0.0", 80, {"example.com"}));
    servers.push_back(createServer("127.0.0.1", 80, {"example.com"}));

    const Config config(servers);
    const Resolver resolver(config);

    // 具体的なアドレスが最優先
    const Address addr("127.0.0.1", 80);
    const auto result = resolver.resolve(addr, "example.com");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), servers[2]);

    // server_name が一致するワイルドカードが次に優先
    const Address addr2("192.168.0.1", 80);
    const auto result2 = resolver.resolve(addr2, "example.com");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap(), servers[1]);

    // どちらも一致しない場合は最初のワイルドカード
    const auto result3 = resolver.resolve(addr2, "unknown.com");
    ASSERT_TRUE(result3.isSome());
    EXPECT_EQ(result3.unwrap(), servers[0]);
}

TEST_F(ResolverTest, ResolveWithPortInHostHeader) {
    ServerContextList servers;
    servers.push_back(createServer("127.0.0.1", 80, {"localhost"}));
    servers.push_back(createServer("127.0.0.1", 80, {"example.com"}));

    const Config config(servers);
    const Resolver resolver(config);

    // ポートは無視される
    const auto result = resolver.resolve(Address("127.0.0.1", 80), "localhost:80");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), servers[0]);

    const auto result2 = resolver.resolve(Address("127.0.0.1", 80), "example.com:80");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap(), servers[1]);
}