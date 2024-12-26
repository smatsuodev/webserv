#include "http/request_parser.hpp"
#include "utils/logger.hpp"

#include <gtest/gtest.h>

using namespace http;

class ParseHeaderFieldLineErr : public testing::Test {
protected:
    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(ParseHeaderFieldLineErr, empty) {
    const auto result = RequestParser::parseHeaderFieldLine("");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseHeaderFieldLineErr, noColon) {
    const auto result = RequestParser::parseHeaderFieldLine("Host");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseHeaderFieldLineErr, emptyFieldName) {
    const auto result = RequestParser::parseHeaderFieldLine(": value");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseHeaderFieldLineErr, spaceBeforeFieldName) {
    const auto result = RequestParser::parseHeaderFieldLine(" Host: value");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseHeaderFieldLineErr, fieldNameContainsSpace) {
    const auto result = RequestParser::parseHeaderFieldLine("Field Name: value");
    EXPECT_TRUE(result.isErr());
}

// RFC 9112: No whitespace is allowed between the field name and colon
TEST_F(ParseHeaderFieldLineErr, spaceBeforeColon) {
    const auto result = RequestParser::parseHeaderFieldLine("Host : value");
    EXPECT_TRUE(result.isErr());
}

class ParseHeaderFieldLineOk : public testing::Test {
protected:
    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(ParseHeaderFieldLineOk, normal) {
    const auto result = RequestParser::parseHeaderFieldLine("Host: example.com");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST_F(ParseHeaderFieldLineOk, noSpaceAfterColon) {
    const auto result = RequestParser::parseHeaderFieldLine("Host:example.com");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST_F(ParseHeaderFieldLineOk, spacesAroundValue) {
    const auto result = RequestParser::parseHeaderFieldLine("Host:   example.com   ");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST_F(ParseHeaderFieldLineOk, tabAroundValue) {
    const auto result = RequestParser::parseHeaderFieldLine("Host:\texample.com\t");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST_F(ParseHeaderFieldLineOk, spaceInValue) {
    const auto result = RequestParser::parseHeaderFieldLine("User-Agent: A browser");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "User-Agent");
    EXPECT_EQ(result.unwrap().second, "A browser");
}

class ParseRequestRequestLineErr : public testing::Test {
protected:
    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(ParseRequestRequestLineErr, empty) {
    const auto result = RequestParser::parseRequest("", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noSpace) {
    const auto result = RequestParser::parseRequest("GET/pathHTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noMethod) {
    const auto result = RequestParser::parseRequest("/path HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noRequestTarget) {
    const auto result = RequestParser::parseRequest("GET HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noHTTPVersion) {
    const auto result = RequestParser::parseRequest("GET /path", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, invalidMethod) {
    const auto result = RequestParser::parseRequest("XXX /path HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, invalidHTTPVersion) {
    const auto result = RequestParser::parseRequest("GET /path XXX/1.0", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, lowerCaseHTTPVersion) {
    const auto result = RequestParser::parseRequest("GET /path http/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noVersionNumber) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noVersionNumber2) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/.", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noMajorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, noMinorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, nonDigitMajorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/X.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, nonDigitMinorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.X", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST_F(ParseRequestRequestLineErr, longVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

class ParseRequestRequestLineOk : public testing::Test {
protected:
    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(ParseRequestRequestLineOk, normal) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isOk());

    const Request expected(kMethodGet, "/path", "HTTP/1.1", {}, "");
    EXPECT_EQ(result.unwrap(), expected);
}

class ParseRequestOk : public testing::Test {
protected:
    void SetUp() override {
        SET_LOG_LEVEL(Logger::kError);
    }
};

TEST_F(ParseRequestOk, normal) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.1", {"Host: example.com"}, "body");
    EXPECT_TRUE(result.isOk());

    const Request expected(kMethodGet, "/path", "HTTP/1.1", {{"Host", "example.com"}}, "body");
    EXPECT_EQ(result.unwrap(), expected);
}
