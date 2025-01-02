#include <gtest/gtest.h>
#include "./utils/reader.hpp"
#include "utils/io/read_buffer.hpp"

void loadAll(ReadBuffer &buffer) {
    while (true) {
        if (const auto loaded = buffer.load().unwrap(); loaded == 0) {
            return;
        }
    }
}

TEST(ByteBufferTest, ConsumeExactBytes) {
    const std::string testData = "Hello, World!";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const std::string result = buffer.consume(5);
    EXPECT_EQ(result, "Hello");
    EXPECT_EQ(buffer.size(), testData.size() - 5);
}

TEST(ByteBufferTest, ConsumeExactBytesExceedBuffer) {
    const std::string testData = "Hello";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const std::string result = buffer.consume(10); // 要求サイズ > 実際のバッファサイズ
    EXPECT_EQ(result, "Hello");
    EXPECT_EQ(buffer.size(), 0);
}

TEST(ByteBufferTest, ConsumeUntilDelimiter) {
    const std::string testData = "key1=value1&key2=value2";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Option<std::string> result = buffer.consumeUntil("&");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), "key1=value1&"); // 区切り文字を含めて返す
    EXPECT_EQ(buffer.size(), testData.size() - result.unwrap().size());
}

TEST(ByteBufferTest, ConsumeUntilDelimiterAtEnd) {
    const std::string testData = "key1=value1&";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Option<std::string> result = buffer.consumeUntil("&");
    ASSERT_TRUE(result.isSome());
    EXPECT_EQ(result.unwrap(), "key1=value1&");
    EXPECT_EQ(buffer.size(), 0);
}

TEST(ByteBufferTest, LoadError) {
    BrokenReader reader;
    ReadBuffer buffer(reader);

    const Result<std::size_t, error::AppError> result = buffer.load();
    ASSERT_TRUE(result.isErr());
}

TEST(ByteBufferTest, ConsumeUntilDelimiterNotFound) {
    const std::string testData = "key1=value1";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Option<std::string> result = buffer.consumeUntil("|");
    ASSERT_TRUE(result.isNone());

    // サイズは変化しない
    EXPECT_EQ(buffer.size(), testData.size());
}

TEST(ByteBufferTest, MultipleConsumes) {
    const std::string testData = "Hello World! Key=Value&AnotherKey=AnotherValue";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const std::string consumeFirst = buffer.consume(12);
    EXPECT_EQ(consumeFirst, "Hello World!");

    const Option<std::string> consumeUntil = buffer.consumeUntil("&");
    ASSERT_TRUE(consumeUntil.isSome());
    EXPECT_EQ(consumeUntil.unwrap(), " Key=Value&");

    // 残りサイズの確認
    EXPECT_EQ(buffer.size(), testData.size() - consumeFirst.size() - consumeUntil.unwrap().size());
}

TEST(ByteBufferTest, PartialConsumeOfDelimiterSequence) {
    const std::string testData = "ABCD|EFGH|";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Option<std::string> consumeUntilFirst = buffer.consumeUntil("|");
    ASSERT_TRUE(consumeUntilFirst.isSome());
    EXPECT_EQ(consumeUntilFirst.unwrap(), "ABCD|");

    const Option<std::string> consumeUntilSecond = buffer.consumeUntil("|");
    ASSERT_TRUE(consumeUntilSecond.isSome());
    EXPECT_EQ(consumeUntilSecond.unwrap(), "EFGH|");

    // バッファが空になっていることを確認
    EXPECT_EQ(buffer.size(), 0);
}
