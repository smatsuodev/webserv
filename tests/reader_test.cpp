#include "utils/io/reader.hpp"
#include "utils/types/result.hpp"
#include <fakeit.hpp>
#include <gtest/gtest.h>

using namespace fakeit;

class StringReader : public io::IReader {
public:
    explicit StringReader(const std::string &data) : data_(data), pos_(0) {}

    ReadResult read(char *buf, std::size_t nbyte) override {
        const std::size_t bytesToRead = std::min(nbyte, data_.size() - pos_);
        std::memcpy(buf, data_.c_str() + pos_, bytesToRead);
        pos_ += bytesToRead;
        return Ok(bytesToRead);
    }

    bool eof() override {
        return pos_ == data_.size();
    }

private:
    std::string data_;
    std::size_t pos_;
};

class BrokenReader : public io::IReader {
public:
    BrokenReader() = default;

    ReadResult read(char *, std::size_t) override {
        return Err<std::string>("Read error");
    }

    bool eof() override {
        return false;
    }
};

// bufio::Reader::read が正常にデータを読み取るか確認
TEST(ReaderTest, ReadSuccess) {
    // bufio::Reader のインスタンスを作成
    const std::string mockData = "Hello, World!";
    StringReader reader(mockData);
    bufio::Reader bufReader(reader);

    // bufio::Reader::read を実行
    char buffer[32];
    const std::size_t bytesToRead = mockData.size();
    const auto result = bufReader.read(buffer, bytesToRead);

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), mockData.size());
    EXPECT_EQ(std::memcmp(buffer, mockData.c_str(), mockData.size()), 0);

    // もう一度呼び出す
    const auto eofResult = bufReader.read(buffer, bytesToRead);
    ASSERT_TRUE(eofResult.isOk());
    EXPECT_EQ(eofResult.unwrap(), 0);
}

// IReader::read を複数回呼ぶ必要がある場合の挙動を確認
TEST(ReaderTest, ReadMultipleTimes) {
    // bufio::Reader のインスタンスを作成
    const std::string mockData = "Hello, World!";
    StringReader reader(mockData);
    bufio::Reader bufReader(reader, 4);

    // 実行
    char buffer[32];
    const std::size_t bytesToRead = mockData.size();
    const auto result = bufReader.read(buffer, bytesToRead);

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), mockData.size());
    EXPECT_EQ(std::memcmp(buffer, mockData.c_str(), mockData.size()), 0);
}

// IReader::read でエラーが発生した場合の挙動を確認
TEST(ReaderTest, ReadError) {
    // bufio::Readerのインスタンスを作成
    BrokenReader reader;
    bufio::Reader bufReader(reader);

    // 実行
    char buffer[8];
    const auto result = bufReader.read(buffer, 8);

    // 結果を検証
    ASSERT_TRUE(result.isErr());
}

// テストケース3: Reader::readUntil がデリミタまで正しく読み取るか確認
TEST(ReaderTest, ReadUntilDelimiter) {
    // インスタンス
    const std::string mockData = "Hello, World! This is a test.";
    StringReader reader(mockData);
    bufio::Reader bufReader(reader, 4);

    // 実行
    const auto result = bufReader.readUntil("!");

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), "Hello, World!");

    // もう一回実行
    const auto result2 = bufReader.readUntil(".");
    ASSERT_TRUE(result2.isOk());
    EXPECT_EQ(result2.unwrap(), " This is a test.");
}

// Reader::readAll がすべてのデータを正しく読み取るか確認
TEST(ReaderTest, ReadAllSuccess) {
    // インスタンス
    const std::string mockData = "All data read successfully.";
    StringReader reader(mockData);
    bufio::Reader bufReader(reader);

    // 実行
    const auto result = bufReader.readAll();

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), mockData);
}

// Reader::peek のテスト
TEST(ReaderTest, Peek) {
    const std::string mockData = "Hello, World!";

    // インスタンス
    StringReader reader(mockData);
    bufio::Reader bufReader(reader);

    // peek
    const auto result = bufReader.peek(5);

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), "Hello");

    // もう一度 peek しても同じ = バッファが消費されていない
    const auto result2 = bufReader.peek(5);

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result2.unwrap(), "Hello");
}