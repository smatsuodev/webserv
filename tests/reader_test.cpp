#include "utils/io/reader.hpp"
#include "utils/types/result.hpp"
#include <fakeit.hpp>
#include <gtest/gtest.h>

using namespace fakeit;

// bufio::Reader::read が正常にデータを読み取るか確認
TEST(ReaderTest, ReadSuccess) {
    // モックの準備
    Mock<io::IReader> mockReader;
    constexpr char mockData[] = "Hello, World!";
    constexpr std::size_t mockDataSize = sizeof(mockData) - 1; // 終端文字を除く
    When(Method(mockReader, read))
        .Do([&](char *buf, std::size_t) -> Result<std::size_t, std::string> {
            std::memcpy(buf, mockData, mockDataSize);
            return Ok(mockDataSize);
        })
        .Return(Ok<size_t>(0));
    When(Method(mockReader, eof)).AlwaysReturn(false);

    // bufio::Readerのインスタンスを作成
    io::IReader &reader = mockReader.get();
    bufio::Reader bufReader(reader);

    // bufio::Reader::read を実行
    char buffer[32];
    constexpr std::size_t bytesToRead = mockDataSize;
    const auto result = bufReader.read(buffer, bytesToRead);

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), mockDataSize);
    EXPECT_EQ(std::memcmp(buffer, mockData, mockDataSize), 0);

    // もう一度呼び出す
    const auto eofResult = bufReader.read(buffer, bytesToRead);
    ASSERT_TRUE(eofResult.isOk());
    EXPECT_EQ(eofResult.unwrap(), 0);
}

// IReader::read を複数回呼ぶ必要がある場合の挙動を確認
TEST(ReaderTest, ReadMultipleTimes) {
    // モックを準備
    Mock<io::IReader> mockReader;
    constexpr char mockData[] = "Hello, World!";
    constexpr std::size_t mockDataSize = sizeof(mockData) - 1;
    std::size_t bytesRead = 0;
    When(Method(mockReader, read))
        .Do([&](char *buf, const std::size_t nbyte) -> Result<std::size_t, std::string> {
            // 高々 5 byte 返す
            const std::size_t bytesToReturn = std::min(nbyte, static_cast<std::size_t>(5));
            std::memcpy(buf, mockData, bytesToReturn);
            bytesRead += bytesToReturn;
            return Ok(bytesToReturn);
        })
        .AlwaysDo([&](char *buf, const std::size_t nbyte) -> Result<std::size_t, std::string> {
            const std::size_t bytesToReturn = std::min(nbyte, mockDataSize - bytesRead);
            std::memcpy(buf, mockData + bytesRead, bytesToReturn);
            bytesRead += bytesToReturn;
            return Ok(bytesToReturn);
        });
    When(Method(mockReader, eof)).AlwaysReturn(false);

    // bufio::Readerのインスタンスを作成
    io::IReader &reader = mockReader.get();
    bufio::Reader bufReader(reader);

    // 実行
    char buffer[32];
    constexpr std::size_t bytesToRead = mockDataSize;
    const auto result = bufReader.read(buffer, bytesToRead);

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), mockDataSize);
    EXPECT_EQ(std::memcmp(buffer, mockData, mockDataSize), 0);

    // モックの呼び出しを検証
    Verify(Method(mockReader, read)).Exactly(2);
    Verify(Method(mockReader, eof)).Never();
}

// IReader::read でエラーが発生した場合の挙動を確認
TEST(ReaderTest, ReadError) {
    // モックを準備
    Mock<io::IReader> mockReader;
    const std::string errorMsg = "Read error";
    When(Method(mockReader, read)).AlwaysReturn(Err<std::string>(errorMsg));
    When(Method(mockReader, eof)).AlwaysReturn(false);

    // bufio::Readerのインスタンスを作成
    io::IReader &reader = mockReader.get();
    bufio::Reader bufReader(reader);

    // 実行
    char buffer[8];
    constexpr std::size_t bytesToRead = 8;
    const auto result = bufReader.read(buffer, bytesToRead);

    // 結果を検証
    ASSERT_TRUE(result.isErr());
    EXPECT_EQ(result.unwrapErr(), errorMsg);
}

// テストケース3: Reader::readUntil がデリミタまで正しく読み取るか確認
TEST(ReaderTest, ReadUntilDelimiter) {
    // モック
    Mock<io::IReader> mockReader;
    constexpr char mockData[] = "Hello, World! This is a test.";
    constexpr std::size_t mockDataSize = sizeof(mockData) - 1;
    size_t bytesRead = 0;
    When(Method(mockReader, read))
        .Do([&](char *buf, const std::size_t nbyte) -> Result<std::size_t, std::string> {
            // 高々 10 byte 返す
            const std::size_t bytesToReturn = std::min(nbyte, static_cast<std::size_t>(10));
            std::memcpy(buf, mockData, bytesToReturn);
            bytesRead += bytesToReturn;
            return Ok(bytesToReturn);
        })
        .AlwaysDo([&](char *buf, const std::size_t nbyte) -> Result<std::size_t, std::string> {
            const std::size_t bytesToReturn = std::min(nbyte, mockDataSize - bytesRead);
            std::memcpy(buf, mockData + bytesRead, bytesToReturn);
            bytesRead += bytesToReturn;
            return Ok(bytesToReturn);
        });
    When(Method(mockReader, eof)).AlwaysReturn(false);

    // インスタンス
    io::IReader &reader = mockReader.get();
    bufio::Reader bufReader(reader);

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
    // モック
    Mock<io::IReader> mockReader;
    constexpr char mockData[] = "All data read successfully.";
    constexpr std::size_t mockDataSize = sizeof(mockData) - 1;
    size_t bytesRead = 0;
    When(Method(mockReader, read))
        .Do([&](char *buf, const std::size_t nbyte) -> Result<std::size_t, std::string> {
            // 高々 10 byte 返す
            const std::size_t bytesToReturn = std::min(nbyte, static_cast<std::size_t>(10));
            std::memcpy(buf, mockData, bytesToReturn);
            bytesRead += bytesToReturn;
            return Ok(bytesToReturn);
        })
        .AlwaysDo([&](char *buf, const std::size_t nbyte) -> Result<std::size_t, std::string> {
            const std::size_t bytesToReturn = std::min(nbyte, mockDataSize - bytesRead);
            std::memcpy(buf, mockData + bytesRead, bytesToReturn);
            bytesRead += bytesToReturn;
            return Ok(bytesToReturn);
        });
    When(Method(mockReader, eof)).AlwaysReturn(false);

    // インスタンス
    io::IReader &reader = mockReader.get();
    bufio::Reader bufReader(reader);

    // 実行
    const auto result = bufReader.readAll();

    // 結果を検証
    ASSERT_TRUE(result.isOk());
    EXPECT_EQ(result.unwrap(), std::string(mockData, mockDataSize));
}
