#include <gtest/gtest.h>
#include "transport/address.hpp"
#include "core/virtual_server_resolver.hpp"
#include "transport/connection.hpp"
#include "utils/logger.hpp"
#include <memory>

using namespace config;

class VirtualServerResolverTest : public ::testing::Test {
protected:
    void SetUp() {
        SET_LOG_LEVEL(Logger::kError);
    }

    static std::unique_ptr<VirtualServer> createVirtualServer(
        const std::string &ip,
        const uint16_t port,
        const std::vector<std::string> &serverName = std::vector<std::string>()
    ) {
        const auto config = ServerContext(ip, port, LocationContextList(), serverName);
        return std::make_unique<VirtualServer>(config, Address(ip, port));
    }

    // Virtual Server のマッチングに必要な情報。localAddr 以外に意味はない
    static Connection createTestConnection(const Address &localAddr) {
        return Connection(-1, localAddr, localAddr);
    }
};

TEST_F(VirtualServerResolverTest, ResolveByAddress) {
    std::vector<std::unique_ptr<VirtualServer> > servers;
    servers.push_back(createVirtualServer("127.0.0.1", 8080));
    servers.push_back(createVirtualServer("127.0.0.1", 8081));

    VirtualServerList vsList;
    for (auto &server : servers) {
        vsList.push_back(server.get());
    }

    const Address addr("127.0.0.1", 8080);
    const auto conn = createTestConnection(addr);
    const VirtualServerResolver resolver(vsList, conn);

    const auto result = resolver.resolve("");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap().get(), *vsList[0]);

    const Address addr2("127.0.0.1", 8081);
    const auto conn2 = createTestConnection(addr2);
    const VirtualServerResolver resolver2(vsList, conn2);
    const auto result2 = resolver2.resolve("");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap().get(), *vsList[1]);

    // 登録していないアドレス
    const Address addr3("192.168.0.1", 8080);
    const auto conn3 = createTestConnection(addr3);
    const VirtualServerResolver resolver3(vsList, conn3);
    const auto result3 = resolver3.resolve("");
    EXPECT_TRUE(result3.isNone());

    // 登録していないポート
    const Address nonExistAddr("127.0.0.1", 9999);
    const auto conn4 = createTestConnection(nonExistAddr);
    const VirtualServerResolver resolver4(vsList, conn4);
    const auto result4 = resolver4.resolve("");
    EXPECT_TRUE(result4.isNone());
}

TEST_F(VirtualServerResolverTest, ResolveByHostHeader) {
    std::vector<std::unique_ptr<VirtualServer> > servers;
    servers.push_back(createVirtualServer("127.0.0.1", 80, {"example.org"}));
    servers.push_back(createVirtualServer("127.0.0.1", 80, {"example.com", "www.example.com"}));

    VirtualServerList vsList;
    for (auto &server : servers) {
        vsList.push_back(server.get());
    }

    const Address addr("127.0.0.1", 80);
    const auto conn = createTestConnection(addr);
    const VirtualServerResolver resolver(vsList, conn);

    // 127.0.0.1:80 は複数あるが、Host が一致するのは後者
    const auto result = resolver.resolve("example.com");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap().get(), *vsList[1]);

    const auto result2 = resolver.resolve("example.org");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap().get(), *vsList[0]);

    // Host が一致しない場合は先頭のものが選択される
    const auto result3 = resolver.resolve("unknown.com");
    ASSERT_TRUE(result3.isSome());
    EXPECT_EQ(result3.unwrap().get(), *vsList[0]);
}

TEST_F(VirtualServerResolverTest, ResolveWithWildcard) {
    std::vector<std::unique_ptr<VirtualServer> > servers;
    servers.push_back(createVirtualServer("127.0.0.1", 80, {"example.org"}));
    servers.push_back(createVirtualServer("0.0.0.0", 80, {"example.com"}));

    VirtualServerList vsList;
    for (auto &server : servers) {
        vsList.push_back(server.get());
    }

    // ワイルドカードより具体的なアドレスが優先される
    const Address addr("127.0.0.1", 80);
    const auto conn = createTestConnection(addr);
    const VirtualServerResolver resolver(vsList, conn);
    const auto result = resolver.resolve("");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap().get(), *vsList[0]);

    // Host を指定しても同様に優先される
    const auto result2 = resolver.resolve("example.com");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap().get(), *vsList[0]);

    // 具体的なアドレスがない場合はワイルドカードにマッチ
    const Address addr2("192.168.0.1", 80);
    const auto conn2 = createTestConnection(addr2);
    const VirtualServerResolver resolver2(vsList, conn2);
    const auto result3 = resolver2.resolve("");
    ASSERT_TRUE(result3.isSome());
    EXPECT_EQ(result3.unwrap().get(), *vsList[1]);
}

TEST_F(VirtualServerResolverTest, ResolveWithMultipleWildcards) {
    std::vector<std::unique_ptr<VirtualServer> > servers;
    servers.push_back(createVirtualServer("0.0.0.0", 80));
    servers.push_back(createVirtualServer("0.0.0.0", 80, {"example.com"}));
    servers.push_back(createVirtualServer("127.0.0.1", 80, {"example.com"}));

    VirtualServerList vsList;
    for (auto &server : servers) {
        vsList.push_back(server.get());
    }

    // 具体的なアドレスが最優先
    const Address addr("127.0.0.1", 80);
    const auto conn = createTestConnection(addr);
    const VirtualServerResolver resolver(vsList, conn);
    const auto result = resolver.resolve("example.com");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap().get(), *vsList[2]);

    // server_name が一致するワイルドカードが次に優先
    const Address addr2("192.168.0.1", 80);
    const auto conn2 = createTestConnection(addr2);
    const VirtualServerResolver resolver2(vsList, conn2);
    const auto result2 = resolver2.resolve("example.com");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap().get(), *vsList[1]);

    // どちらも一致しない場合は最初のワイルドカード
    const auto result3 = resolver2.resolve("unknown.com");
    ASSERT_TRUE(result3.isSome());
    EXPECT_EQ(result3.unwrap().get(), *vsList[0]);
}

TEST_F(VirtualServerResolverTest, ResolveWithPortInHostHeader) {
    std::vector<std::unique_ptr<VirtualServer> > servers;
    servers.push_back(createVirtualServer("127.0.0.1", 80, {"localhost"}));
    servers.push_back(createVirtualServer("127.0.0.1", 80, {"example.com"}));

    VirtualServerList vsList;
    for (auto &server : servers) {
        vsList.push_back(server.get());
    }

    const Address addr("127.0.0.1", 80);
    const auto conn = createTestConnection(addr);
    const VirtualServerResolver resolver(vsList, conn);

    // ポートは無視される
    const auto result = resolver.resolve("localhost:80");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap().get(), *vsList[0]);

    const auto result2 = resolver.resolve("example.com:80");
    ASSERT_TRUE(result2.isSome());
    EXPECT_EQ(result2.unwrap().get(), *vsList[1]);
}

// config がホスト名で書かれている場合
TEST_F(VirtualServerResolverTest, ResolveWithConfigHost) {
    VirtualServer vs(ServerContext("localhost", 8080, LocationContextList()), Address("127.0.0.1", 8080));
    // virtual server は解決済みのアドレスを受け取る
    VirtualServerList vsList = {&vs};

    const Address addr("127.0.0.1", 8080);
    const auto conn = createTestConnection(addr);
    const VirtualServerResolver resolver(vsList, conn);

    const auto result = resolver.resolve("localhost");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap().get(), vs);
}