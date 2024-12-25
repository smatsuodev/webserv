#include <gtest/gtest.h>
#include "http/status.hpp"

using namespace http;

TEST(HttpStatusCodeFromIntTest, OK) {
    EXPECT_EQ(httpStatusCodeFromInt(200), kStatusOk);
}

TEST(GetHttpStatusTextTest, OK) {
    EXPECT_EQ(getHttpStatusText(kStatusOk), "OK");
}
