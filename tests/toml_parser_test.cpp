#include <gtest/gtest.h>
#include "config/toml/parser.hpp"

using namespace toml;

void testOk(const std::string &text, const Table &expected) {
    const TomlParser::ParseResult result = TomlParser(Tokenizer(text).tokenize().unwrap()).parse();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), expected);
}

void testErr(const std::string &text) {
    const TomlParser::ParseResult result = TomlParser(Tokenizer(text).tokenize().unwrap()).parse();
    ASSERT_TRUE(result.isErr());
}

TEST(ParserOk, intKeyValues) {
    const auto text = R"(
    int1 = 42
    int2 = +42
    int3 = -42
    int4 = 0
    )";
    // NOTE: この format スタイルちょっと微妙
    const auto expected = Table({
        std::make_pair("int1", Value(42l)),
        std::make_pair("int2", Value(+42l)),
        std::make_pair("int3", Value(-42l)),
        std::make_pair("int4", Value(0l)),
    });

    const TomlParser::ParseResult result = TomlParser(Tokenizer(text).tokenize().unwrap()).parse();
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap().getValue("int1"), Some(Value(42l)));
    testOk(text, expected);
}

TEST(ParserOk, boolKeyValues) {
    const auto text = R"(
    b1 = true
    b2 = false
    )";
    const auto expected = Table({
        std::make_pair("b1", Value(true)),
        std::make_pair("b2", Value(false)),
    });

    testOk(text, expected);
}

TEST(ParserOk, stringKeyValues) {
    const auto text = R"(
    s1 = "string"
    s2 = "string with space"
    s3 = "ゆにこーど"
    's4' = "single quoted key"
    "s5" = "double quoted key"
    )";
    const auto expected = Table({
        std::make_pair("s1", Value("string")),
        std::make_pair("s2", Value("string with space")),
        std::make_pair("s3", Value("ゆにこーど")),
        std::make_pair("s4", Value("single quoted key")),
        std::make_pair("s5", Value("double quoted key")),
    });

    testOk(text, expected);
}

TEST(ParserOk, keyValues) {
    const auto text = R"(
    array = [1,2]
    point = { x = 0, y = 1 }
    )";
    // NOTE: この format スタイルちょっと微妙
    const auto expected = Table(
        {std::make_pair("array", Value(Array({Value(1l), Value(2l)}))),
         std::make_pair("point", Value(Table({std::make_pair("x", Value(0l)), std::make_pair("y", Value(1l))})))}
    );

    testOk(text, expected);
}

TEST(ParserOk, dottedKey) {
    const auto text = R"(
    fruit.count = 2

    fruit.apple.color = "red"
    fruit.apple.price = 100

    fruit.orange.color = "orange"
    fruit.orange.price = 200
    )";

    const auto appleTable = Table({std::make_pair("color", Value("red")), std::make_pair("price", Value(100l))});
    const auto orangeTable = Table({std::make_pair("color", Value("orange")), std::make_pair("price", Value(200l))});
    const auto expected = Table({std::make_pair(
        "fruit",
        Value(Table({
            std::make_pair("count", Value(2l)),
            std::make_pair("apple", Value(appleTable)),
            std::make_pair("orange", Value(orangeTable)),
        }))
    )});

    testOk(text, expected);
}

TEST(ParserOkDiscouraged, messyDottedKey) {
    const auto text = R"(
    fruit.count = 2

    # fruit.*.color を代入
    fruit.apple.color = "red"
    fruit.orange.color = "orange"

    # fruit.*.price を代入
    fruit.apple.price = 100
    fruit.orange.price = 200
    )";

    const auto appleTable = Table({std::make_pair("color", Value("red")), std::make_pair("price", Value(100l))});
    const auto orangeTable = Table({std::make_pair("color", Value("orange")), std::make_pair("price", Value(200l))});
    const auto expected = Table({std::make_pair(
        "fruit",
        Value(Table({
            std::make_pair("count", Value(2l)),
            std::make_pair("apple", Value(appleTable)),
            std::make_pair("orange", Value(orangeTable)),
        }))
    )});

    testOk(text, expected);
}

// TEST(ParserOk, array) {
//     const auto text = R"(
//     array = [1,2,]
//     nested = [[1, ], [2, ]]
//     deep = [[[1]]]
//     mix = [1, true, "string", [1], { x = 1 }]
//     )";
//
//     const auto array = Array({Value(1l), Value(2l)});
//     const auto nested = Array({Value(Array({Value(1l)})), Value(Array({Value(2l)}))});
//     const auto deep = Array({Value(Array({Value(Array({Value(1l)}))}))});
//     const auto mix = Array(
//         {Value(1l),
//          Value(true),
//          Value("string"),
//          Value(Array({Value(1l)})),
//          Value(Table({std::make_pair("x", Value(1l))}))}
//     );
//     const auto expected = Table({
//         std::make_pair("array", Value(array)),
//         std::make_pair("nested", Value(nested)),
//         std::make_pair("deep", Value(deep)),
//         std::make_pair("mix", Value(mix)),
//     });
//
//     testOk(text, expected);
// }
//
// TEST(ParserOk, table) {
//     const auto text = R"(
//     [table]
//     x = 1
//     [table.sub]
//     y = 1
//     [other]
//     z = 1
//     )";
//     const auto table = Table({
//         std::make_pair("x", Value(1l)),
//         std::make_pair("sub", Value(Table({std::make_pair("y", Value(1l))}))),
//     });
//     const auto expected = Table({
//         std::make_pair("table", Value(table)),
//         std::make_pair("other", Value(Table({std::make_pair("z", Value(1l))}))),
//     });
//
//     testOk(text, expected);
// }
//
// TEST(ParserOk, addSubTable) {
//     const auto text = R"(
//     foo.bar = 1
//     [foo.baz] # foo.baz は定義されていないのでOK
//     x = 1
//     )";
//     const auto fooTable = Table({
//         std::make_pair("bar", Value(1l)),
//         std::make_pair("baz", Value(Table({std::make_pair("x", Value(1l))}))),
//     });
//     const auto expected = Table({
//         std::make_pair("foo", Value(fooTable)),
//     });
//
//     testOk(text, expected);
// }
//
// TEST(ParserOkDiscouraged, messyTable) {
//     const auto text = R"(
//     [foo.bar]
//     [x]
//     [foo.baz]
//     )";
//     const auto expected = Table({
//         std::make_pair(
//             "foo", Value(Table({std::make_pair("bar", Value(Table())), std::make_pair("baz", Value(Table()))}))
//         ),
//         std::make_pair("x", Value(Table())),
//     });
//
//     testOk(text, expected);
// }
//
// TEST(ParserOk, arrayOfTable) {
//     const auto text = R"(
//     [[server]]
//     port = 80
//     [[server]]
//     port = 3000
//     [[server]]  # empty
//     )";
//     const auto expected = Table({std::make_pair(
//         "server",
//         Value(Array({
//             Value(Table({std::make_pair("port", Value(80l))})),
//             Value(Table({std::make_pair("port", Value(3000l))})),
//             Value(Table()),
//         }))
//     )});
//
//     testOk(text, expected);
// }
//
// TEST(ParserOk, arrayOfNestedTable) {
//     const auto text = R"(
//     [[server]]
//     port = 80
//     [server.error_page]
//     404 = "404.html"
//     [server.foo]
//     x = 1
//     )";
//     const auto errorPage = Table({std::make_pair("404", Value("404.html"))});
//     const auto table = Table({
//         std::make_pair("port", Value(80l)),
//         std::make_pair("error_page", Value(errorPage)),
//         std::make_pair("foo", Value(Table({std::make_pair("x", Value(1l))}))),
//     });
//     const auto expected = Table({std::make_pair("server", Value(Array({Value(table)})))});
//
//     testOk(text, expected);
// }

TEST(ParserErr, exprInSameLine) {
    const auto text = R"(
    key = 0 key = 0
    )";
    testErr(text);
}

TEST(ParserErr, duplicateKey) {
    const auto text = R"(
    key = 0
    key = 0 # 同じキー
    )";

    testErr(text);
}

TEST(ParserErr, duplicateKeyQuoted) {
    const auto text = R"(
    key = 0
    "key" = 0 # quote しても同じキー
    )";


    testErr(text);
}

// TEST(ParserErr, editInlineTable) {
//     const auto text = R"(
//     key = { x = 1 }
//     key.y = 2 # key は inline table なので、上書き不可
//     )";
//
//     testErr(text);
// }
//
// TEST(ParserErr, editTableByInlineTable) {
//     const auto text = R"(
//     [key]
//     foo.bar = 1
//     foo = { baz = 2 } # 既存の table は inline table で上書き不可
//     )";
//
//     testErr(text);
// }
//
// TEST(ParserErr, sameTable) {
//     const auto text = R"(
//     [table]
//     x = 1
//     [table] # 同じテーブル
//     y = 1
//     )";
//
//
//     testErr(text);
// }
//
// TEST(ParserErr, addByTable) {
//     const auto text = R"(
//     [foo]
//     bar.x = 1
//     [foo.bar] # 既存の foo.bar に table を再定義不可
//     y = 1
//     )";
//     testErr(text);
// }
//
// TEST(ParserErr, overwriteByTable) {
//     const auto text = R"(
//     [table]
//     x = 1
//     [table.x]   # x は既に決まってる
//     y = 1
//     )";
//
//     testErr(text);
// }
//
// TEST(ParserErr, overwriteTableByArrayOfTable) {
//     const auto text = R"(
//     [foo]
//     x = 1
//     [[foo]] # 既に foo は table なので不可
//     y = 1
//     )";
//
//     testErr(text);
// }
//
// TEST(ParserErr, overwriteArrayByArrayOfTable) {
//     const auto text = R"(
//     a = []
//     [[a]] # a は既に配列
//     )";
//
//     testErr(text);
// }
//
// TEST(ParserErr, overwriteArrayOfTableByTableNested) {
//     const auto text = R"(
//     [[array]]
//     [[array.nestedArray]]
//     [array.nestedArray] # nestedArray は既に配列
//     )";
//
//     testErr(text);
// }
