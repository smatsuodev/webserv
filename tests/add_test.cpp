#include <fakeit.hpp>
#include <gtest/gtest.h>
#include "add.hpp"

using namespace fakeit;

TEST(Small, OnePlusTwo) {
    EXPECT_EQ(add(1, 2), 3);
}

TEST(Large, IntMax) {
    EXPECT_EQ(add(INT_MAX, 0), INT_MAX);
}
