#ifndef TESTS_UTILS_READER_HPP
#define TESTS_UTILS_READER_HPP

#include "utils/io/reader.hpp"

class StringReader : public io::IReader {
public:
    explicit StringReader(const std::string &data);
    ReadResult read(char *buf, std::size_t nbyte) override;
    bool eof() override;

private:
    std::string data_;
    std::size_t pos_;
};

// 常にエラーを返す
class BrokenReader : public io::IReader {
public:
    ReadResult read(char *, std::size_t) override;
    bool eof() override;
};

// n 回目に kWouldBlock を返す
class WouldBlockReader : public io::IReader {
public:
    WouldBlockReader(IReader &reader, int n);
    ReadResult read(char *buf, std::size_t nbyte);
    bool eof() override;

private:
    IReader &reader_;
    int n_;
    int counter_;
};

// 交互に kWouldBlock を返す
class WouldBlockReader2 : public io::IReader {
public:
    explicit WouldBlockReader2(IReader &reader);
    ReadResult read(char *buf, std::size_t nbyte) override;
    bool eof() override;

private:
    IReader &reader_;
};

#endif
