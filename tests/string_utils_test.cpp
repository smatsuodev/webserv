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

TEST(StoulErr, emptyStr) {
    const auto result = utils::stoul("");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, nonDigit) {
    const auto result = utils::stoul("a");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, leadingSpace) {
    const auto result = utils::stoul(" 123");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, trailingSpace) {
    const auto result = utils::stoul("123 ");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, leadingAndTrailingSpace) {
    const auto result = utils::stoul(" 123 ");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, leadingMinus) {
    const auto result = utils::stoul("-123");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, leadingPlus) {
    const auto result = utils::stoul("+123");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulErr, overflow) {
    constexpr auto ulMax = std::numeric_limits<unsigned long>::max();
    const auto result = utils::stoul(std::to_string(ulMax) + "0");
    EXPECT_TRUE(result.isErr());
}

TEST(StoulOk, zero) {
    const auto result = utils::stoul("0");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(0, result.unwrap());
}

TEST(StoulOk, one) {
    const auto result = utils::stoul("1");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(1, result.unwrap());
}

TEST(StoulOk, normal) {
    const auto result = utils::stoul("123");
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(123, result.unwrap());
}

TEST(StoulOk, max) {
    constexpr auto ulMax = std::numeric_limits<unsigned long>::max();
    const auto result = utils::stoul(std::to_string(ulMax));
    EXPECT_TRUE(result.isOk());
    EXPECT_EQ(ulMax, result.unwrap());
}
