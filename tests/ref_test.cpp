#include <gtest/gtest.h>
#include "utils/ref.hpp"

TEST(Ref, write) {
    int x = 0;
    const Ref ref = x;
    ref.get() = 42;
    EXPECT_EQ(x, 42);
}

TEST(Ref, read) {
    int x = 0;
    const Ref ref = x;
    EXPECT_EQ(ref.get(), 0);

    // 通常の ref で書いた値が反映されることを確認
    int &stdRef = x;
    stdRef = 42;
    EXPECT_EQ(ref.get(), 42);
}

TEST(Ref, changeRef) {
    int x = 0, y = 0;
    Ref ref = x;

    ref.get() = 42;
    ref = y;
    ref.get() = 42;

    EXPECT_EQ(x, 42);
    EXPECT_EQ(y, 42);
}