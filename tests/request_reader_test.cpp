#include <gtest/gtest.h>
#include <fakeit.hpp>
#include "./utils/reader.hpp"
#include "../src/lib/http/request/reader/request_reader.hpp"
#include "utils/logger.hpp"
#include "config/config.hpp"
#include <utility>

using namespace fakeit;
using namespace config;
using namespace http;

class RequestReaderTest : public testing::Test {
public:
    RequestReaderTest() {
        const ServerContext config = ServerContext("0.0.0.0", 80, {});
        When(Method(resolverMock_, resolve)).AlwaysReturn(Some(config));
        reqReader_ = std::make_unique<RequestReader>(resolverMock_.get());
    }

protected:
    Mock<IConfigResolver> resolverMock_;
    std::unique_ptr<RequestReader> reqReader_;

    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(RequestReaderTest, GetNormal) {
    const std::string request = "GET / HTTP/1.1\r\n"
                                "Host: example.com\r\n\r\n";
    StringReader reader(request);
    ReadBuffer readBuf(reader);

    const auto result = reqReader_->readRequest(readBuf);
    EXPECT_TRUE(result.isOk());

    const Request expected = Request(
        kMethodGet,
        "/",
        "HTTP/1.1",
        {
            std::make_pair("Host", "example.com"),
        }
    );
    EXPECT_EQ(result.unwrap().unwrap(), expected);
}

TEST_F(RequestReaderTest, PostNormal) {
    const std::string request = "POST / HTTP/1.1\r\n"
                                "Content-Length: 5\r\n"
                                "Host: example.com\r\n"
                                "\r\n"
                                "hello";
    StringReader reader(request);
    ReadBuffer readBuf(reader);

    const auto result = reqReader_->readRequest(readBuf);
    EXPECT_TRUE(result.isOk());

    const Request expected = Request(
        kMethodPost,
        "/",
        "HTTP/1.1",
        {
            std::make_pair("Content-Length", "5"),
            std::make_pair("Host", "example.com"),
        },
        "hello"
    );
    EXPECT_EQ(result.unwrap().unwrap(), expected);
}

TEST_F(RequestReaderTest, GetWouldBlock) {
    const std::string request = "GET / HTTP/1.1\r\n"
                                "Host: example.com\r\n\r\n";
    StringReader sReader(request);
    WouldBlockReader2 reader(sReader);
    ReadBuffer readBuf(reader);

    // Request が得られるまで繰り返す
    auto result = reqReader_->readRequest(readBuf);
    uint count = 20;
    while (result.isErr() || result.unwrap().isNone()) {
        result = reqReader_->readRequest(readBuf);
        ASSERT_GE(count--, 0);
    }

    const Request expected = Request(
        kMethodGet,
        "/",
        "HTTP/1.1",
        {
            std::make_pair("Host", "example.com"),
        }
    );
    const auto unwrapped = result.unwrap().unwrap();
    EXPECT_EQ(unwrapped, expected);
}

TEST_F(RequestReaderTest, PostWouldBlock) {
    const std::string request = "POST / HTTP/1.1\r\n"
                                "Content-Length: 5\r\n"
                                "Host: example.com\r\n"
                                "\r\n"
                                "hello";
    StringReader sReader(request);
    WouldBlockReader2 reader(sReader);
    ReadBuffer readBuf(reader);

    // Request が得られるまで繰り返す
    auto result = reqReader_->readRequest(readBuf);
    uint count = 20;
    while (result.isErr() || result.unwrap().isNone()) {
        result = reqReader_->readRequest(readBuf);
        ASSERT_GE(count--, 0);
    }

    const Request expected = Request(
        kMethodPost,
        "/",
        "HTTP/1.1",
        {
            std::make_pair("Content-Length", "5"),
            std::make_pair("Host", "example.com"),
        },
        "hello"
    );
    EXPECT_EQ(result.unwrap().unwrap(), expected);
}

TEST_F(RequestReaderTest, PostChunked) {
    const std::string request = "POST / HTTP/1.1\r\n"
                                "Transfer-Encoding: chunked\r\n"
                                "Host: example.com\r\n"
                                "\r\n"
                                "5\r\n"
                                "hello\r\n"
                                "6\r\n"
                                " world\r\n"
                                "0\r\n"
                                "\r\n";
    StringReader reader(request);
    ReadBuffer readBuf(reader);

    const auto result = reqReader_->readRequest(readBuf);
    ASSERT_TRUE(result.isOk());

    const Request expected = Request(
        kMethodPost,
        "/",
        "HTTP/1.1",
        {
            std::make_pair("Transfer-Encoding", "chunked"),
            std::make_pair("Host", "example.com"),
        },
        "hello world"
    );
    EXPECT_EQ(result.unwrap().unwrap(), expected);
}

// TODO: chunked のテストをもっと増やす
