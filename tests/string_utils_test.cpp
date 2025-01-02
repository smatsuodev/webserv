#include "utils/string.hpp"
#include <gtest/gtest.h>

TEST(ToStringTest, IntToString) {
    constexpr int value = 123;
    const std::string expected = "123";
    const std::string actual = utils::toString(value);

    ASSERT_EQ(expected, actual);
}

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

TEST(Join, empty) {
    const std::vector<std::string> ss;
    const auto result = utils::join(ss, " ");
    EXPECT_EQ(result, "");
}

TEST(Join, single) {
    const std::vector<std::string> ss = {"a"};
    const auto result = utils::join(ss, " ");
    EXPECT_EQ(result, "a");
}

TEST(Join, many) {
    const std::vector<std::string> ss = {"a", "b", "c", "d"};
    const auto result = utils::join(ss, " | ");
    EXPECT_EQ(result, "a | b | c | d");
}

TEST(Join, emptyElements) {
    const std::vector<std::string> ss = {"", "", ""};
    const auto result = utils::join(ss, "x");
    EXPECT_EQ(result, "xx");
}

TEST(StrnstrSome, emptyStr) {
    const char *s = "";
    const auto result = utils::strnstr(s, "", 0);
    EXPECT_TRUE(result.isSome());
    // アドレスの比較
    EXPECT_EQ(s, result.unwrap());
}

TEST(StrnstrSome, emptyNeedle) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, "", 0);
    EXPECT_TRUE(result.isSome());
    // アドレスの比較
    EXPECT_EQ(s, result.unwrap());
}

TEST(StrnstrSome, emptyStrAndNeedle) {
    const char *s = "";
    const auto result = utils::strnstr(s, "", 0);
    EXPECT_TRUE(result.isSome());
    // アドレスの比較
    EXPECT_EQ(s, result.unwrap());
}

TEST(StrnstrSome, hasNeedle) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, "b", 3);
    EXPECT_TRUE(result.isSome());
    // アドレスの比較
    EXPECT_EQ(s + 1, result.unwrap());
}

TEST(StrnstrNone, noNeedle) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, "d", 3);
    EXPECT_TRUE(result.isNone());
}

TEST(StrnstrNone, needleLongerThanStr) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, "abcd", 3);
    EXPECT_TRUE(result.isNone());
}

TEST(StrnstrNone, shortLen) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, "abc", 2);
    EXPECT_TRUE(result.isNone());
}

TEST(StrnstrNone, zeroLen) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, "abc", 0);
    EXPECT_TRUE(result.isNone());
}

TEST(StrnstrNone, nullStr) {
    const char *s = nullptr;
    const auto result = utils::strnstr(s, "abc", 3);
    EXPECT_TRUE(result.isNone());
}

TEST(StrnstrNone, nullNeedle) {
    const char *s = "abc";
    const auto result = utils::strnstr(s, nullptr, 3);
    EXPECT_TRUE(result.isNone());
}

TEST(Trim, empty) {
    const auto result = utils::trim("");
    EXPECT_EQ(result, "");
}

TEST(Trim, spaces) {
    const auto result = utils::trim("\t\n\v\f\r ");
    EXPECT_EQ(result, "");
}

TEST(Trim, normal) {
    const auto result = utils::trim("\t\n\v\f\r word\t\n\v\f\r ");
    EXPECT_EQ(result, "word");
}

TEST(Split, empty) {
    const auto result = utils::split("", '.');
    const std::vector<std::string> expected = {""};
    EXPECT_EQ(result, expected);
}

TEST(Split, onlyDelimiter) {
    const auto result = utils::split(".", '.');
    const std::vector<std::string> expected = {"", ""};
    EXPECT_EQ(result, expected);
}

TEST(Split, delimiters) {
    const auto result = utils::split("...", '.');
    const std::vector<std::string> expected = {"", "", "", ""};
    EXPECT_EQ(result, expected);
}

TEST(Split, singleWord) {
    const auto result = utils::split("word", '.');
    const std::vector<std::string> expected = {"word"};
    EXPECT_EQ(result, expected);
}

TEST(Split, wordAround) {
    const auto result = utils::split(".word.", '.');
    const std::vector<std::string> expected = {"", "word", ""};
    EXPECT_EQ(result, expected);
}

TEST(Split, normal) {
    const auto result = utils::split("a.b.c", '.');
    const std::vector<std::string> expected = {"a", "b", "c"};
    EXPECT_EQ(result, expected);
}
