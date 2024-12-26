#include "utils/string.hpp"
#include <gtest/gtest.h>

TEST(StartsWith, emptyStr) {
    EXPECT_TRUE(utils::startsWith("", ""));
    EXPECT_FALSE(utils::startsWith("", "a"));
}

TEST(StartsWith, emptyPrefix) {
    EXPECT_TRUE(utils::startsWith("a", ""));
    EXPECT_TRUE(utils::startsWith("abc", ""));
}

TEST(StartsWith, hasPrefix) {
    EXPECT_TRUE(utils::startsWith("a", "a"));
    EXPECT_TRUE(utils::startsWith("abc", "abc"));
}

TEST(StartsWith, noPrefix) {
    EXPECT_FALSE(utils::startsWith("a", "b"));
    EXPECT_FALSE(utils::startsWith("abc", "def"));
}

TEST(StartsWith, prefixLongerThanStr) {
    EXPECT_FALSE(utils::startsWith("a", "ab"));
    EXPECT_FALSE(utils::startsWith("abc", "abcdef"));
}

TEST(EndsWith, emptyStr) {
    EXPECT_TRUE(utils::endsWith("", ""));
    EXPECT_FALSE(utils::endsWith("", "a"));
}

TEST(EndsWith, emptySuffix) {
    EXPECT_TRUE(utils::endsWith("a", ""));
    EXPECT_TRUE(utils::endsWith("abc", ""));
}

TEST(EndsWith, hasSuffix) {
    EXPECT_TRUE(utils::endsWith("a", "a"));
    EXPECT_TRUE(utils::endsWith("abc", "abc"));
}

TEST(EndsWith, noSuffix) {
    EXPECT_FALSE(utils::endsWith("a", "b"));
    EXPECT_FALSE(utils::endsWith("abc", "def"));
}

TEST(EndsWith, suffixLongerThanStr) {
    EXPECT_FALSE(utils::endsWith("a", "ab"));
    EXPECT_FALSE(utils::endsWith("abc", "abcdef"));
}
