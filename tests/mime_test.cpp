#include <gtest/gtest.h>
#include "http/mime.hpp"

using namespace http;

TEST(GetMimeTypeTest, empty) {
    const auto result = getMimeType("");
    EXPECT_EQ(result, "application/octet-stream");
}

TEST(GetMimeTypeTest, noExt) {
    const auto result = getMimeType("foo");
    EXPECT_EQ(result, "application/octet-stream");

    const auto result2 = getMimeType("foo.");
    EXPECT_EQ(result2, "application/octet-stream");

    const auto result3 = getMimeType("txt");
    EXPECT_EQ(result3, "application/octet-stream");
}

TEST(GetMimeTypeTest, onlyDot) {
    const auto result = getMimeType(".");
    EXPECT_EQ(result, "application/octet-stream");
}

TEST(GetMimeTypeTest, doubleDot) {
    const auto result = getMimeType("..");
    EXPECT_EQ(result, "application/octet-stream");

    const auto result2 = getMimeType("..txt");
    EXPECT_EQ(result2, "text/plain");

    const auto result3 = getMimeType("txt..");
    EXPECT_EQ(result3, "application/octet-stream");
}

TEST(GetMimeTypeTest, multiDot) {
    const auto result = getMimeType("txt.png.html");
    EXPECT_EQ(result, "text/html");
}

TEST(GetMimeTypeTest, UPPERCASE) {
    const auto result = getMimeType("foo.TXT");
    EXPECT_EQ(result, "text/plain");
}

TEST(GetMimeTypeTest, AnY_Case) {
    const auto result = getMimeType("foo.TxT");
    EXPECT_EQ(result, "text/plain");
}

TEST(GetMimeTypeTest, similar) {
    const auto result = getMimeType("txtx");
    EXPECT_EQ(result, "application/octet-stream");
}
