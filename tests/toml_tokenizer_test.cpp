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

TEST(TokenizerOk, numericSymbol) {
    const auto text = R"(42)";
    testOk(text, {Token(kSymbol, "42"), Token(kEof, "")});
}

TEST(TokenizerOk, alphabeticSymbol) {
    const auto text = R"(key)";
    testOk(text, {Token(kSymbol, "key"), Token(kEof, "")});
}

TEST(TokenizerOk, symbolStartWithNumberEndWithAlphabet) {
    const auto text = R"(42key)";
    testOk(text, {Token(kSymbol, "42key"), Token(kEof, "")});
}

TEST(TokenizerOk, symbolStartWithAlphabetEndWithNumber) {
    const auto text = R"(key42)";
    testOk(text, {Token(kSymbol, "key42"), Token(kEof, "")});
}

TEST(TokenizerOk, symbolWithUnderscore) {
    const auto text = R"(_key_42_)";
    testOk(text, {Token(kSymbol, "_key_42_"), Token(kEof, "")});
}

TEST(TokenizerOk, singleQuotedSymbol) {
    const auto text = R"('ke"y')";
    testOk(text, {Token(kQuotedSymbol, "ke\"y"), Token(kEof, "")});
}

TEST(TokenizerOk, doubleQuotedSymbol) {
    const auto text = R"("ke'y")";
    testOk(text, {Token(kQuotedSymbol, "ke'y"), Token(kEof, "")});
}

TEST(TokenizerOk, commentAfterValue) {
    const auto text = R"(key = "value" # comment)";
    testOk(
        text,
        {
            Token(kSymbol, "key"),
            Token(kAssignment, "="),
            Token(kQuotedSymbol, "value"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, notComment) {
    const auto text = R"(key = "# this is not comment")";
    testOk(
        text,
        {
            Token(kSymbol, "key"),
            Token(kAssignment, "="),
            Token(kQuotedSymbol, "# this is not comment"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, keyValue) {
    const auto text = R"(key = 0)";
    testOk(
        text,
        {
            Token(kSymbol, "key"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, intKey) {
    const auto text = R"(123 = 0)";
    testOk(
        text,
        {
            Token(kSymbol, "123"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, quotedKey) {
    const auto text = R"("127.0.0.1" = 0)";
    testOk(
        text,
        {
            Token(kQuotedSymbol, "127.0.0.1"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, keyContainsEq) {
    const auto text = R"("a=b" = 0)";
    testOk(
        text,
        {
            Token(kQuotedSymbol, "a=b"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, unicodeKey) {
    const auto text = R"("ʎǝʞ" = 0)";
    testOk(
        text,
        {
            Token(kQuotedSymbol, "ʎǝʞ"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, singleQuotedKey) {
    const auto text = R"('key' = 0)";
    testOk(
        text,
        {
            Token(kQuotedSymbol, "key"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}

TEST(TokenizerOk, quotedQuoteKey) {
    const auto text = R"('"quoted key"' = 0)";
    testOk(
        text,
        {
            Token(kQuotedSymbol, R"("quoted key")"),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
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
            Token(kQuotedSymbol, ""),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
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
            Token(kQuotedSymbol, ""),
            Token(kAssignment, "="),
            Token(kSymbol, "0"),
            Token(kEof, ""),
        }
    );
}
//
// TEST(TokenizerOk, dottedKey) {
//     const auto text = R"(k1.k2 = 0)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedQuotedKey) {
//     const auto text = R"(k1."k2.k3".k4 = 0)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, spaceAroudDot) {
//     const auto text = R"(k1 . k2 = 0)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, notFloat) {
//     const auto text = R"(3 . 14 = "pi")";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, plusInteger) {
//     const auto text = R"(k = +99)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, Integer) {
//     const auto text = R"(k = 42)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, integerZero) {
//     const auto text = R"()";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, minusInteger) {
//     const auto text = R"(k = -17)";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, emptyArray) {
//     const auto text = R"(k = [])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, intArray) {
//     const auto text = R"(k = [1, 2])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, stringArray) {
//     const auto text = R"(k = ["foo", 'bar'])";
//     testOk(text, {});
// }
//
//
// TEST(TokenizerOk, nestedArray) {
//     const auto text = R"(k = [[1, 2], [3, 4]])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, nestedMixedArray) {
//     const auto text = R"(k = [[1, 2], ["a", "b"]])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, mixedArray) {
//     const auto text = R"(k = [1, "value", [1], { key = "value" }, ])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, justStringArray) {
//     const auto text = R"(k1.k2 = ["[1]", "{a.b=0}"])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, table) {
//     const auto text = R"([table])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedTable) {
//     const auto text = R"([a.b.c])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedSpaceTable) {
//     const auto text = R"([ a.b.c ])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedSpaceTable2) {
//     const auto text = R"([ a . b . c ])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedSpaceTable3) {
//     const auto text = R"([ j . "ʞ" . 'l' ])";
//     testOk(text, {});
// }
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
//
// TEST(TokenizerOk, arrayOfTable) {
//     const auto text = R"([[array]])";
//     testOk(text, {});
// }
//
// TEST(TokenizerOk, dottedArrayOfTable) {
//     const auto text = R"([[a.b]])";
//     testOk(text, {});
// }
//

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
