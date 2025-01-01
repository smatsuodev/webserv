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

    const Result<std::string, error::AppError> result = buffer.consume(5);
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), "Hello");
    EXPECT_EQ(buffer.size(), testData.size() - 5);
}

TEST(ByteBufferTest, ConsumeExactBytesExceedBuffer) {
    const std::string testData = "Hello";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Result<std::string, error::AppError> result = buffer.consume(10); // 要求サイズ > 実際のバッファサイズ
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), "Hello");
    EXPECT_EQ(buffer.size(), 0);
}

TEST(ByteBufferTest, ConsumeUntilDelimiter) {
    const std::string testData = "key1=value1&key2=value2";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Result<Option<std::string>, error::AppError> result = buffer.consumeUntil("&");
    ASSERT_TRUE(result.isOk());
    ASSERT_TRUE(result.unwrap().isSome());
    EXPECT_EQ(result.unwrap().unwrap(), "key1=value1&"); // 区切り文字を含めて返す
    EXPECT_EQ(buffer.size(), testData.size() - result.unwrap().unwrap().size());
}

TEST(ByteBufferTest, ConsumeUntilDelimiterAtEnd) {
    const std::string testData = "key1=value1&";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Result<Option<std::string>, error::AppError> result = buffer.consumeUntil("&");
    ASSERT_TRUE(result.isOk());
    ASSERT_TRUE(result.unwrap().isSome());
    EXPECT_EQ(result.unwrap().unwrap(), "key1=value1&");
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

    const Result<Option<std::string>, error::AppError> result = buffer.consumeUntil("|");
    ASSERT_TRUE(result.isOk());
    ASSERT_TRUE(result.unwrap().isNone());

    // サイズは変化しない
    EXPECT_EQ(buffer.size(), testData.size());
}

TEST(ByteBufferTest, MultipleConsumes) {
    const std::string testData = "Hello World! Key=Value&AnotherKey=AnotherValue";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Result<std::string, error::AppError> consumeFirst = buffer.consume(12);
    ASSERT_TRUE(consumeFirst.isOk());
    EXPECT_EQ(consumeFirst.unwrap(), "Hello World!");

    const Result<Option<std::string>, error::AppError> consumeUntil = buffer.consumeUntil("&");
    ASSERT_TRUE(consumeUntil.isOk());
    ASSERT_TRUE(consumeUntil.unwrap().isSome());
    EXPECT_EQ(consumeUntil.unwrap().unwrap(), " Key=Value&");

    // 残りサイズの確認
    EXPECT_EQ(buffer.size(), testData.size() - consumeFirst.unwrap().size() - consumeUntil.unwrap().unwrap().size());
}

TEST(ByteBufferTest, PartialConsumeOfDelimiterSequence) {
    const std::string testData = "ABCD|EFGH|";
    StringReader reader(testData);
    ReadBuffer buffer(reader);

    loadAll(buffer);
    EXPECT_EQ(buffer.size(), testData.size());

    const Result<Option<std::string>, error::AppError> consumeUntilFirst = buffer.consumeUntil("|");
    ASSERT_TRUE(consumeUntilFirst.isOk());
    ASSERT_TRUE(consumeUntilFirst.unwrap().isSome());
    EXPECT_EQ(consumeUntilFirst.unwrap().unwrap(), "ABCD|");

    const Result<Option<std::string>, error::AppError> consumeUntilSecond = buffer.consumeUntil("|");
    ASSERT_TRUE(consumeUntilSecond.isOk());
    ASSERT_TRUE(consumeUntilSecond.unwrap().isSome());
    EXPECT_EQ(consumeUntilSecond.unwrap().unwrap(), "EFGH|");

    // バッファが空になっていることを確認
    EXPECT_EQ(buffer.size(), 0);
}
