#include "http/method.hpp"
#include <gtest/gtest.h>

using namespace http;

TEST(HttpMethodFromString, get) {
    EXPECT_EQ(httpMethodFromString("GET"), kMethodGet);
}

TEST(HttpMethodFromString, post) {
    EXPECT_EQ(httpMethodFromString("POST"), kMethodPost);
}

TEST(HttpMethodFromString, put) {
    EXPECT_EQ(httpMethodFromString("PUT"), kMethodPut);
}

TEST(HttpMethodFromString, delete_) {
    EXPECT_EQ(httpMethodFromString("DELETE"), kMethodDelete);
}

TEST(HttpMethodFromString, head) {
    EXPECT_EQ(httpMethodFromString("HEAD"), kMethodHead);
}

TEST(HttpMethodFromString, options) {
    EXPECT_EQ(httpMethodFromString("OPTIONS"), kMethodOptions);
}

TEST(HttpMethodFromString, trace) {
    EXPECT_EQ(httpMethodFromString("TRACE"), kMethodTrace);
}

TEST(HttpMethodFromString, connect) {
    EXPECT_EQ(httpMethodFromString("CONNECT"), kMethodConnect);
}

TEST(HttpMethodFromString, patch) {
    EXPECT_EQ(httpMethodFromString("PATCH"), kMethodPatch);
}

TEST(HttpMethodFromString, unknown) {
    EXPECT_EQ(httpMethodFromString("UNKNOWN"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("get"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("post"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("put"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("delete"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("head"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("options"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("trace"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("connect"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString("patch"), kMethodUnknown);
    EXPECT_EQ(httpMethodFromString(""), kMethodUnknown);
}
