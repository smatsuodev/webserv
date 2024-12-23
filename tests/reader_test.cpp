#include "utils/io/reader.hpp"
#include "utils/types/result.hpp"
#include <fakeit.hpp>
#include <gtest/gtest.h>

using namespace fakeit;

class StringReader : public io::IReader {
public:
    explicit StringReader(const std::string &data) : data_(data), pos_(0) {}

    ReadResult read(char *buf, const std::size_t nbyte) override {
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
        return Err(error::kUnknown);
    }

    bool eof() override {
        return false;
    }
};

class WouldBlockReader : public io::IReader {
public:
    // n 回目に error::kIOWouldBlock を返す
    WouldBlockReader(io::IReader &reader, const int n) : reader_(reader), n_(n), counter_(0) {}

    ReadResult read(char *buf, const std::size_t nbyte) {
        if (++counter_ == n_) {
            return Err(error::kIOWouldBlock);
        }
        return reader_.read(buf, nbyte);
    }

    bool eof() {
        return reader_.eof();
    }

private:
    io::IReader &reader_;
    int n_;
    int counter_;
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

// Reader::discard のテスト
TEST(ReaderTest, Discard) {
    const std::string mockData = "Hello, World!";

    // インスタンス
    StringReader reader(mockData);
    bufio::Reader bufReader(reader);

    // discard
    const auto result = bufReader.discard(5);

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), 5);

    // 残りを読み取る
    const auto restResult = bufReader.readAll();

    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(restResult.unwrap(), ", World!");
}

// 途中で EWOULDBLOCK が発生しても、正しく読み込める
TEST(ReaderTest, WouldBlockReadAll) {
    const std::string mockData = "Hello, World!";

    StringReader sReader(mockData);
    // 2回目に EWOULDBLOCK が発生する
    WouldBlockReader reader(sReader, 2);
    // 2回以上のreadが必要なバッファサイズにする
    bufio::Reader bufReader(reader, 4);

    const auto result = bufReader.readAll();

    // 途中で EWOULDBLOCK が発生し、エラーになる
    ASSERT_TRUE(result.isErr());

    // リトライ後は成功する
    const auto result2 = bufReader.readAll();

    ASSERT_TRUE(result2.isOk());
    EXPECT_EQ(result2.unwrap(), mockData);
}

TEST(ReaderTest, WouldBlockReadAllWithLargeBuf) {
    const std::string mockData = "Hello, World!";

    StringReader sReader(mockData);
    WouldBlockReader reader(sReader, 2);
    bufio::Reader bufReader(reader);

    const auto result = bufReader.readAll();

    // 途中で EWOULDBLOCK が発生し、エラーになる
    ASSERT_TRUE(result.isErr());

    // リトライ後は成功する
    const auto result2 = bufReader.readAll();

    ASSERT_TRUE(result2.isOk());
    EXPECT_EQ(result2.unwrap(), mockData);
}

TEST(ReaderTest, WouldBlockReaderRead) {
    const std::string mockData = "Hello";

    StringReader sReader(mockData);
    WouldBlockReader reader(sReader, 2);
    bufio::Reader bufReader(reader, 1);

    char buf[6] = {0};
    const auto r = bufReader.read(buf, 5);

    // 途中で EWOULDBLOCK が起こる
    ASSERT_TRUE(r.isErr());

    const auto r2 = bufReader.read(buf, 5);

    ASSERT_TRUE(r2.isOk());
    ASSERT_STREQ(buf, "Hello");
}

TEST(ReaderTest, WouldBlockReaderReadWithLargeBuf) {
    const std::string mockData = "Hello";

    StringReader sReader(mockData);
    WouldBlockReader reader(sReader, 2);
    bufio::Reader bufReader(reader);

    char buf[6] = {0};
    const auto r2 = bufReader.read(buf, 5);

    ASSERT_TRUE(r2.isOk());
    ASSERT_STREQ(buf, "Hello");
}

TEST(ReaderTest, WouldBlockReaderReadUntil) {
    const std::string mockData = "first line\nsecond line\n";

    StringReader sReader(mockData);
    WouldBlockReader reader(sReader, 2);
    bufio::Reader bufReader(reader, 4);

    const auto r = bufReader.readUntil("\n");

    // 途中で EWOULDBLOCK
    ASSERT_TRUE(r.isErr());

    const auto r2 = bufReader.readUntil("\n");
    const auto r3 = bufReader.readUntil("\n");

    ASSERT_TRUE(r2.isOk());
    ASSERT_TRUE(r3.isOk());
    ASSERT_EQ(r2.unwrap(), "first line\n");
    ASSERT_EQ(r3.unwrap(), "second line\n");
}

TEST(ReaderTest, WouldBlockReaderReadUntilWithLargeBuf) {
    const std::string mockData = "first line\nsecond line\n";

    StringReader sReader(mockData);
    WouldBlockReader reader(sReader, 2);
    bufio::Reader bufReader(reader);

    const auto r2 = bufReader.readUntil("\n");
    const auto r3 = bufReader.readUntil("\n");

    ASSERT_TRUE(r2.isOk());
    ASSERT_TRUE(r3.isOk());
    ASSERT_EQ(r2.unwrap(), "first line\n");
    ASSERT_EQ(r3.unwrap(), "second line\n");
}
