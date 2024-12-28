#include "config/toml/tokenizer.hpp"
#include <gtest/gtest.h>

using namespace toml;

TEST(TokenizerOk, comment) {
    const auto text = R"(# comment)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, commentAfterValue) {
    const auto text = R"(key = "value" # comment)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, notComment) {
    const auto text = R"(key = "# this is not comment")";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, keyValue) {
    const auto text = R"(key = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, intKey) {
    const auto text = R"(123 = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, quotedKey) {
    const auto text = R"("127.0.0.1" = 0)";
    EXPECT_TRUE(false);
}

// "a=b" = 0
TEST(TokenizerOk, keyContainsEq) {
    const auto text = R"("a=b" = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, unicodeKey) {
    const auto text = R"("ʎǝʞ" = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, singleQuotedKey) {
    const auto text = R"('key' = 0)";
    EXPECT_TRUE(false);
}

// 'quoted "value"' = 0
TEST(TokenizerOk, quotedQuoteKey) {
    const auto text = R"('"quoted key"' = 0)";
    EXPECT_TRUE(false);
}

// "" = "blank"
TEST(TokenizerOk, emptyKey) {
    const auto text = R"("" = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, emptyKeySingle) {
    const auto text = R"('' = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedKey) {
    const auto text = R"(k1.k2 = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedQuotedKey) {
    const auto text = R"(k1."k2.k3".k4 = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, spaceAroudDot) {
    const auto text = R"(k1 . k2 = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, notFloat) {
    const auto text = R"(3 . 14 = "pi")";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, plusInteger) {
    const auto text = R"(k = +99)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, Integer) {
    const auto text = R"(k = 42)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, integerZero) {
    const auto text = R"()";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, minusInteger) {
    const auto text = R"(k = -17)";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, emptyArray) {
    const auto text = R"(k = [])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, intArray) {
    const auto text = R"(k = [1, 2])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, stringArray) {
    const auto text = R"(k = ["foo", 'bar'])";
    EXPECT_TRUE(false);
}


TEST(TokenizerOk, nestedArray) {
    const auto text = R"(k = [[1, 2], [3, 4]])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, nestedMixedArray) {
    const auto text = R"(k = [[1, 2], ["a", "b"]])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, mixedArray) {
    const auto text = R"(k = [1, "value", [1], { key = "value" }, ])";
    EXPECT_TRUE(false);
}


TEST(TokenizerOk, justStringArray) {
    const auto text = R"(k1.k2 = ["[1]", "{a.b=0}"])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, table) {
    const auto text = R"([table])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedTable) {
    const auto text = R"([a.b.c])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedSpaceTable) {
    const auto text = R"([ a.b.c ])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedSpaceTable2) {
    const auto text = R"([ a . b . c ])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedSpaceTable3) {
    const auto text = R"([ j . "ʞ" . 'l' ])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, inlineTable) {
    const auto text = R"(k = {x=0, y=1})";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedKeyInInlineTable) {
    const auto text = R"(k = { a.b = 0 })";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, arrayOfTable) {
    const auto text = R"([[array]])";
    EXPECT_TRUE(false);
}

TEST(TokenizerOk, dottedArrayOfTable) {
    const auto text = R"([[a.b]])";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, noValue) {
    const auto text = R"(k = )";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, noKey) {
    const auto text = R"( = 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, multipleEq) {
    const auto text = R"(k == 0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, multipleKeyValue) {
    const auto text = R"(k1=0 k2=0)";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, emptyInlineTableComma) {
    const auto text = R"(k = {, })";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, emptyArrayComma) {
    const auto text = R"(k = [, ])";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, inlineTableTrailingComma) {
    const auto text = R"(k = { a=1, })";
    EXPECT_TRUE(false);
}

TEST(TokenizerErr, invalidTable) {
    const auto text = R"([[[table]]])";
    EXPECT_TRUE(false);
}
