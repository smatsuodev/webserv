#include "config/toml/tokenizer.hpp"
#include <gtest/gtest.h>

using namespace toml;

void testOk(const std::string &text, const Tokenizer::Tokens &expected) {
    const Tokenizer::TokenizeResult result = Tokenizer(text).tokenize();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), expected);
}

void testErr(const std::string &text) {
    const Tokenizer::TokenizeResult result = Tokenizer(text).tokenize();
    ASSERT_TRUE(result.isErr());
}

TEST(TokenizerOk, empty) {
    const auto text = R"()";
    testOk(text, {Token(kEof, "")});
}

TEST(TokenizerOk, comment) {
    const auto text = R"(# comment)";
    testOk(text, {Token(kEof, "")});
}

TEST(TokenizerOk, integer) {
    const auto text = R"(42)";
    testOk(text, {Token(kNum, "42"), Token(kEof, "")});
}

TEST(TokenizerOk, key) {
    const auto text = R"(key)";
    testOk(text, {Token(kKey, "key"), Token(kEof, "")});
}

TEST(TokenizerOk, keyStartWithNumberEndWithAlphabet) {
    const auto text = R"(42key)";
    testOk(text, {Token(kKey, "42key"), Token(kEof, "")});
}

TEST(TokenizerOk, keyStartWithAlphabetEndWithNumber) {
    const auto text = R"(key42)";
    testOk(text, {Token(kKey, "key42"), Token(kEof, "")});
}

TEST(TokenizerOk, keyWithUnderscore) {
    const auto text = R"(_key_42_)";
    testOk(text, {Token(kKey, "_key_42_"), Token(kEof, "")});
}

TEST(TokenizerOk, singleQuotedString) {
    const auto text = R"('ke"y')";
    testOk(text, {Token(kString, "ke\"y"), Token(kEof, "")});
}

TEST(TokenizerOk, doubleQuotedString) {
    const auto text = R"("ke'y")";
    testOk(text, {Token(kString, "ke'y"), Token(kEof, "")});
}

TEST(TokenizerOk, commentAfterValue) {
    const auto text = R"(key = "value" # comment)";
    testOk(
        text,
        {
            Token(kKey, "key"),
            Token(kAssignment, "="),
            Token(kString, "value"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, notComment) {
    const auto text = R"(key = "# this is not comment")";
    testOk(
        text,
        {
            Token(kKey, "key"),
            Token(kAssignment, "="),
            Token(kString, "# this is not comment"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, keyValue) {
    const auto text = R"(key = 0)";
    testOk(
        text,
        {
            Token(kKey, "key"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, intKey) {
    const auto text = R"(123 = 0)";
    testOk(
        text,
        {
            Token(kNum, "123"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, quotedKey) {
    const auto text = R"("127.0.0.1" = 0)";
    testOk(
        text,
        {
            Token(kString, "127.0.0.1"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, keyContainsEq) {
    const auto text = R"("a=b" = 0)";
    testOk(
        text,
        {
            Token(kString, "a=b"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, unicodeKey) {
    const auto text = R"("ʎǝʞ" = 0)";
    testOk(
        text,
        {
            Token(kString, "ʎǝʞ"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, singleQuotedKey) {
    const auto text = R"('key' = 0)";
    testOk(
        text,
        {
            Token(kString, "key"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, quotedQuoteKey) {
    const auto text = R"('"quoted key"' = 0)";
    testOk(
        text,
        {
            Token(kString, R"("quoted key")"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

// 有効だが非推奨
TEST(TokenizerOkDiscouraged, emptyKey) {
    const auto text = R"("" = 0)";
    testOk(
        text,
        {
            Token(kString, ""),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}
//
// 有効だが非推奨
TEST(TokenizerOkDiscouraged, emptyKeySingle) {
    const auto text = R"('' = 0)";
    testOk(
        text,
        {
            Token(kString, ""),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, dottedKey) {
    const auto text = R"(k1.k2 = 0)";
    testOk(
        text,
        {
            Token(kKey, "k1"),
            Token(kDot, "."),
            Token(kKey, "k2"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, dottedQuotedKey) {
    const auto text = R"(k1."k2.k3".k4 = 0)";
    testOk(
        text,
        {Token(kKey, "k1"),
         Token(kDot, "."),
         Token(kString, "k2.k3"),
         Token(kDot, "."),
         Token(kKey, "k4"),
         Token(kAssignment, "="),
         Token(kNum, "0"),
         Token(kEof, "")}
    );
}

TEST(TokenizerOk, spaceAroudDot) {
    const auto text = R"(k1 . k2 = 0)";
    testOk(
        text,
        {
            Token(kKey, "k1"),
            Token(kDot, "."),
            Token(kKey, "k2"),
            Token(kAssignment, "="),
            Token(kNum, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, notFloat) {
    const auto text = R"(3 . 14 = "pi")";
    testOk(
        text,
        {
            Token(kNum, "3"),
            Token(kDot, "."),
            Token(kNum, "14"),
            Token(kAssignment, "="),
            Token(kString, "pi"),
            Token(kEof, ""),
        }
    );
}

// TEST(TokenizerOk, plusInteger) {
//     const auto text = R"(k = +99)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, minusInteger) {
//     const auto text = R"(k = -17)";
//     testOk(text, {});
// }

TEST(TokenizerOk, emptyArray) {
    const auto text = R"([])";
    testOk(text, {Token(kLBracket, "["), Token(kRBracket, "]"), Token(kEof, "")});
}

TEST(TokenizerOk, intArray) {
    const auto text = R"([1, 2])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kNum, "1"),
            Token(kComma, ","),
            Token(kNum, "2"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, stringArray) {
    const auto text = R"(["foo", 'bar'])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kString, "foo"),
            Token(kComma, ","),
            Token(kString, "bar"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}


TEST(TokenizerOk, nestedArray) {
    const auto text = R"([[1, 2], [3, 4]])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kLBracket, "["),
            Token(kNum, "1"),
            Token(kComma, ","),
            Token(kNum, "2"),
            Token(kRBracket, "]"),
            Token(kComma, ","),
            Token(kLBracket, "["),
            Token(kNum, "3"),
            Token(kComma, ","),
            Token(kNum, "4"),
            Token(kRBracket, "]"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, nestedMixedArray) {
    const auto text = R"([[1, 2], ["a", "b"]])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kLBracket, "["),
            Token(kNum, "1"),
            Token(kComma, ","),
            Token(kNum, "2"),
            Token(kRBracket, "]"),
            Token(kComma, ","),
            Token(kLBracket, "["),
            Token(kString, "a"),
            Token(kComma, ","),
            Token(kString, "b"),
            Token(kRBracket, "]"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

// TEST(TokenizerOk, mixedArray) {
//     const auto text = R"(k = [1, "value", [1], { key = "value" }, ])";
//     testOk(text, {});
// }

TEST(TokenizerOk, justStringArray) {
    const auto text = R"(k1.k2 = ["[1]", "{a.b=0}"])";
    testOk(
        text,
        {
            Token(kKey, "k1"),
            Token(kDot, "."),
            Token(kKey, "k2"),
            Token(kAssignment, "="),
            Token(kLBracket, "["),
            Token(kString, "[1]"),
            Token(kComma, ","),
            Token(kString, "{a.b=0}"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, table) {
    const auto text = R"([table])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kKey, "table"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, dottedTable) {
    const auto text = R"([a.b.c])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kKey, "a"),
            Token(kDot, "."),
            Token(kKey, "b"),
            Token(kDot, "."),
            Token(kKey, "c"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, dottedSpaceTable) {
    const auto text = R"([ a.b.c ])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kKey, "a"),
            Token(kDot, "."),
            Token(kKey, "b"),
            Token(kDot, "."),
            Token(kKey, "c"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, dottedSpaceTable2) {
    const auto text = R"([ a . b . c ])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kKey, "a"),
            Token(kDot, "."),
            Token(kKey, "b"),
            Token(kDot, "."),
            Token(kKey, "c"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}
//
TEST(TokenizerOk, dottedSpaceTable3) {
    const auto text = R"([ j . "ʞ" . 'l' ])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kKey, "j"),
            Token(kDot, "."),
            Token(kString, "ʞ"),
            Token(kDot, "."),
            Token(kString, "l"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}
//
// TEST(TokenizerOk, inlineTable) {
//     const auto text = R"(k = {x=0, y=1})";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedKeyInInlineTable) {
//     const auto text = R"(k = { a.b = 0 })";
//     testOk(text, {});
// }

TEST(TokenizerOk, arrayOfTable) {
    const auto text = R"([[array]])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kLBracket, "["),
            Token(kKey, "array"),
            Token(kRBracket, "]"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, dottedArrayOfTable) {
    const auto text = R"([[a.b]])";
    testOk(
        text,
        {
            Token(kLBracket, "["),
            Token(kLBracket, "["),
            Token(kKey, "a"),
            Token(kDot, "."),
            Token(kKey, "b"),
            Token(kRBracket, "]"),
            Token(kRBracket, "]"),
            Token(kEof, ""),
        }
    );
}


TEST(TokenizerErr, invalidChar) {
    const auto text = R"(%)";
    testErr(text);
}

// TEST(TokenizerErr, noValue) {
//     const auto text = R"(k = )";
//     testErr(text);
// }
//
// TEST(TokenizerErr, noKey) {
//     const auto text = R"( = 0)";
//     testErr(text);
// }
//
// TEST(TokenizerErr, multipleEq) {
//     const auto text = R"(k == 0)";
//     testErr(text);
// }
//
// TEST(TokenizerErr, multipleKeyValue) {
//     const auto text = R"(k1=0 k2=0)";
//     testErr(text);
// }
//
// TEST(TokenizerErr, emptyInlineTableComma) {
//     const auto text = R"(k = {, })";
//     testErr(text);
// }
//
// TEST(TokenizerErr, emptyArrayComma) {
//     const auto text = R"(k = [, ])";
//     testErr(text);
// }
//
// TEST(TokenizerErr, inlineTableTrailingComma) {
//     const auto text = R"(k = { a=1, })";
//     testErr(text);
// }
//
// TEST(TokenizerErr, invalidTable) {
//     const auto text = R"([[[table]]])";
//     testErr(text);
// }
