#include "http/request_parser.hpp"
#include "utils/logger.hpp"

#include <gtest/gtest.h>

using namespace http;

TEST(ParseHeaderFieldLineErr, empty) {
    const auto result = RequestParser::parseHeaderFieldLine("");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseHeaderFieldLineErr, noColon) {
    const auto result = RequestParser::parseHeaderFieldLine("Host");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseHeaderFieldLineErr, emptyFieldName) {
    const auto result = RequestParser::parseHeaderFieldLine(": value");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseHeaderFieldLineErr, spaceBeforeFieldName) {
    const auto result = RequestParser::parseHeaderFieldLine(" Host: value");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseHeaderFieldLineErr, fieldNameContainsSpace) {
    const auto result = RequestParser::parseHeaderFieldLine("Field Name: value");
    EXPECT_TRUE(result.isErr());
}

// RFC 9112: No whitespace is allowed between the field name and colon
TEST(ParseHeaderFieldLineErr, spaceBeforeColon) {
    const auto result = RequestParser::parseHeaderFieldLine("Host : value");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseHeaderFieldLineOk, normal) {
    const auto result = RequestParser::parseHeaderFieldLine("Host: example.com");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST(ParseHeaderFieldLineOk, noSpaceAfterColon) {
    const auto result = RequestParser::parseHeaderFieldLine("Host:example.com");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST(ParseHeaderFieldLineOk, spacesAroundValue) {
    const auto result = RequestParser::parseHeaderFieldLine("Host:   example.com   ");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST(ParseHeaderFieldLineOk, tabAroundValue) {
    const auto result = RequestParser::parseHeaderFieldLine("Host:\texample.com\t");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "Host");
    EXPECT_EQ(result.unwrap().second, "example.com");
}

TEST(ParseHeaderFieldLineOk, spaceInValue) {
    const auto result = RequestParser::parseHeaderFieldLine("User-Agent: A browser");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().first, "User-Agent");
    EXPECT_EQ(result.unwrap().second, "A browser");
}

TEST(ParseRequestRequestLineErr, empty) {
    const auto result = RequestParser::parseRequest("", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noSpace) {
    const auto result = RequestParser::parseRequest("GET/pathHTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noMethod) {
    const auto result = RequestParser::parseRequest("/path HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noRequestTarget) {
    const auto result = RequestParser::parseRequest("GET HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noHTTPVersion) {
    const auto result = RequestParser::parseRequest("GET /path", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, invalidMethod) {
    const auto result = RequestParser::parseRequest("XXX /path HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, invalidHTTPVersion) {
    const auto result = RequestParser::parseRequest("GET /path XXX/1.0", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, lowerCaseHTTPVersion) {
    const auto result = RequestParser::parseRequest("GET /path http/1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noVersionNumber) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noVersionNumber2) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/.", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noMajorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, noMinorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, nonDigitMajorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/X.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, nonDigitMinorVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.X", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineErr, longVersion) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.1.1", {}, "");
    EXPECT_TRUE(result.isErr());
}

TEST(ParseRequestRequestLineOk, normal) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.1", {}, "");
    EXPECT_TRUE(result.isOk());

    const Request expected(kMethodGet, "/path", "HTTP/1.1", {}, "");
    EXPECT_EQ(result.unwrap(), expected);
}

TEST(ParseRequestOk, normal) {
    const auto result = RequestParser::parseRequest("GET /path HTTP/1.1", {"Host: example.com"}, "body");
    EXPECT_TRUE(result.isOk());

    const Request expected(kMethodGet, "/path", "HTTP/1.1", {{"Host", "example.com"}}, "body");
    EXPECT_EQ(result.unwrap(), expected);
}
