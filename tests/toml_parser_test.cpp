#include <gtest/gtest.h>
#include "config/toml/parser.hpp"
#include "config/toml/value.hpp"

using namespace toml;

TEST(ParserOk, keyValues) {
    const auto text = R"(
    key = "value"
    b1 = true
    b2 = false
    int1 = 42
    int2 = +42
    int3 = -42
    int4 = 0
    array = [1,2]
    point = { x = 0, y = 1 }
    )";
    // NOTE: インデントを維持するために空のコメントを入れてる
    const auto expected = Table({
        //
        std::make_pair("key", Value("value")),
        //
        std::make_pair("b1", Value(true)),
        //
        std::make_pair("b2", Value(false)),
        //
        std::make_pair("int1", Value(42l)),
        //
        std::make_pair("int2", Value(+42l)),
        //
        std::make_pair("int3", Value(-42l)),
        //
        std::make_pair("int4", Value(0l)),
        //
        std::make_pair("array", Value(Array({Value(1l), Value(2l)}))),
        //
        std::make_pair("point", Value(Table({std::make_pair("x", Value(0l)), std::make_pair("y", Value(1l))})))
        //
    });

    EXPECT_TRUE(false);
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
    const auto expected = //
        Table({std::make_pair("fruit",
                              Value(Table({
                                  std::make_pair("count", Value(2l)),
                                  std::make_pair("apple", Value(appleTable)),
                                  std::make_pair("orange", Value(orangeTable)),
                              })))});
    EXPECT_TRUE(false);
}

TEST(ParseOkDiscouraged, messyDottedKey) {
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
    const auto expected = //
        Table({std::make_pair("fruit",
                              Value(Table({
                                  std::make_pair("count", Value(2l)),
                                  std::make_pair("apple", Value(appleTable)),
                                  std::make_pair("orange", Value(orangeTable)),
                              })))});
    EXPECT_TRUE(false);
}

TEST(ParseOk, array) {
    const auto text = R"(
    array = [1,2,]
    nested = [[1, ], [2, ]]
    deep = [[[1]]]
    mix = [1, true, "string", [1], { x = 1 }]
    )";

    const auto array = Array({Value(1l), Value(2l)});
    const auto nested = Array({Value(Array({Value(1l)})), Value(Array({Value(2l)}))});
    const auto deep = Array({Value(Array({Value(Array({Value(1l)}))}))});
    const auto mix = Array( //
        {//
         Value(1l),
         //
         Value(true),
         //
         Value("string"),
         //
         Value(Array({Value(1l)})),
         //
         Value(Table({std::make_pair("x", Value(1l))}))});
    const auto expected = Table({
        std::make_pair("array", Value(array)),
        std::make_pair("nested", Value(nested)),
        std::make_pair("deep", Value(deep)),
        std::make_pair("mix", Value(mix)),
    });
    EXPECT_TRUE(false);
}

TEST(ParserOk, table) {
    const auto text = R"(
    [table]
    x = 1
    [table.sub]
    y = 1
    [other]
    z = 1
    )";
    const auto table = Table({
        std::make_pair("x", Value(1l)),
        std::make_pair("sub", Value(Table({std::make_pair("y", Value(1l))}))),
    });
    const auto expected = Table({
        //
        std::make_pair("table", Value(table)),
        //
        std::make_pair("other", Value(Table({std::make_pair("z", Value(1l))}))),
    });

    EXPECT_TRUE(false);
}

TEST(ParserOk, addSubTable) {
    const auto text = R"(
    foo.bar = 1
    [foo.baz] # foo.baz は定義されていないのでOK
    x = 1
    )";
    const auto fooTable = Table({
        //
        std::make_pair("bar", Value(1l)),
        //
        std::make_pair("baz", Value(Table({std::make_pair("x", Value(1l))}))),
    });
    const auto expected = Table({
        std::make_pair("foo", Value(fooTable)),
    });

    EXPECT_TRUE(false);
}

TEST(ParserErr, duplicateKey) {
    const auto text = R"(
    key = 0
    key = 0 # 同じキー
    )";

    EXPECT_TRUE(false);
}

TEST(ParserErr, duplicateKeyQuoted) {
    const auto text = R"(
    key = 0
    "key" = 0 # quote しても同じキー
    )";

    EXPECT_TRUE(false);
}

TEST(ParserErr, editInlineTable) {
    const auto text = R"(
    key = { x = 1 }
    key.y = 2 # key は inline table なので、上書き不可
    )";
    EXPECT_TRUE(false);
}

TEST(ParserErr, editTableByInlineTable) {
    const auto text = R"(
    [key]
    foo.bar = 1
    foo = { baz = 2 } # 既存の table は inline table で上書き不可
    )";
    EXPECT_TRUE(false);
}

TEST(ParserErr, sameTable) {
    const auto text = R"(
    [table]
    x = 1
    [table] # 同じテーブル
    y = 1
    )";

    EXPECT_TRUE(false);
}

TEST(ParserErr, addByTable) {
    const auto text = R"(
    [foo]
    bar.x = 1
    [foo.bar] # 既存の foo.bar に table を再定義不可
    y = 1
    )";
}

TEST(ParserErr, overwriteByTable) {
    const auto text = R"(
    [table]
    x = 1
    [table.x]   # x は既に決まってる
    y = 1
    )";

    EXPECT_TRUE(false);
}
