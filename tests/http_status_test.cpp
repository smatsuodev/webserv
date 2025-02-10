#include <gtest/gtest.h>
#include "http/status.hpp"

using namespace http;

TEST(HttpStatusCodeFromIntTest, OK) {
    EXPECT_EQ(httpStatusCodeFromInt(200), Some(kStatusOk));
}

TEST(HttpStatusCodeFromIntTest, Invalid) {
    EXPECT_EQ(httpStatusCodeFromInt(999), None);
}

TEST(GetHttpStatusTextTest, OK) {
    EXPECT_EQ(getHttpStatusText(kStatusOk), "OK");
}
