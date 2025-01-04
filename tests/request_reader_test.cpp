#include <gtest/gtest.h>
#include "./utils/reader.hpp"
#include "../src/lib/core/handler/request_reader.hpp"
#include "utils/logger.hpp"

#include <utility>

class RequestReaderTest : public ::testing::Test {
protected:
    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(RequestReaderTest, GetNormal) {
    const std::string request = "GET / HTTP/1.1\r\n"
                                "Host: example.com\r\n\r\n";
    StringReader reader(request);
    ReadBuffer readBuf(reader);
    RequestReader reqReader(readBuf);

    const auto result = reqReader.readRequest();
    EXPECT_TRUE(result.isOk());

    const http::Request expected = http::Request(
        http::kMethodGet,
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
    RequestReader reqReader(readBuf);

    const auto result = reqReader.readRequest();
    EXPECT_TRUE(result.isOk());

    const http::Request expected = http::Request(
        http::kMethodPost,
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
    RequestReader reqReader(readBuf);

    // Request が得られるまで繰り返す
    auto result = reqReader.readRequest();
    uint count = 20;
    while (result.isErr() || result.unwrap().isNone()) {
        result = reqReader.readRequest();
        ASSERT_GE(count--, 0);
    }

    const http::Request expected = http::Request(
        http::kMethodGet,
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
    RequestReader reqReader(readBuf);

    // Request が得られるまで繰り返す
    auto result = reqReader.readRequest();
    uint count = 20;
    while (result.isErr() || result.unwrap().isNone()) {
        result = reqReader.readRequest();
        ASSERT_GE(count--, 0);
    }

    const http::Request expected = http::Request(
        http::kMethodPost,
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
